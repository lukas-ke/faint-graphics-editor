// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#include <array>
#include <sstream>
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "formats/gif/file-gif.hh"
#include "geo/int-rect.hh"
#include "util/enum-util.hh"
#include "util/image-props.hh"
#include "util-wx/file-path.hh"
#include "util-wx/stream.hh"

namespace faint{

class LoadGifError{
  // Exception thrown for failures in gif-loading. Not meant to be
  // propagated outside load_gif.
public:
  LoadGifError(const utf8_string& error)
    : m_error(error)
  {}

  utf8_string GetString() const{
    return m_error;
  }

private:
  utf8_string m_error;
};

class Alphabet{
public:
  Alphabet(int codeSize)
    : codeSize(codeSize),
      clearCode(1 << codeSize),
      endOfStreamCode(clearCode + 1)
  {
    Reset();
    stackPos = 0;
  }

  bool Empty() const{
    return stackPos == -1;
  }

  int Pop(){
    return stack[to_size_t(stackPos--)];
  }

  void Push(int code){
    stack[to_size_t(stackPos)] = code;
    stackPos++;
  }

  void ResetStack(){
    stackPos = 0;
  }

  void Set(int code){
    stack[to_size_t(stackPos)] = code;
  }

  int BuildString(int code){
    if (stackPos >= Alphabet::SIZE){
      throw LoadGifError("stackPos >= Alphabet::SIZE");
    }

    while(code > clearCode){
      stack[to_size_t(stackPos++)] = tail[to_size_t(code)];
      code = prefix[to_size_t(code)];

      if (stackPos >= Alphabet::SIZE){
        throw LoadGifError("stackPos >= Alphabet::SIZE");
      }
    }
    return code;
  }

  void NewEntry(int code, int lastCode){
    if (freeIndex > maxIndex){
      // Missing clear code, I suppose this could be a "deferred clear
      // code" as described in GIF89a spec. Returning seems to work OK.
      return;
    }
    assert(freeIndex < Alphabet::SIZE);
    prefix[to_size_t(freeIndex)] = lastCode;
    tail[to_size_t(freeIndex)] = code;
    freeIndex++;

    if (freeIndex > maxIndex && bits < 12){
      bits++;
      maxIndex = (1 << bits) - 1;
    }
  }

  void Reset(){
    bits = codeSize + 1;
    freeIndex = (1 << codeSize) + 2;
    maxIndex = (1 << bits) - 1;
  }

  static const int SIZE = 4096 + 1;
  std::array<int, SIZE> prefix;
  std::array<int, SIZE> tail;
  std::array<int, SIZE> stack;

  const int codeSize;
  const int clearCode;
  const int endOfStreamCode;
  int stackPos;
  int bits;
  int freeIndex;
  int maxIndex;
private:
  Alphabet& operator=(const Alphabet&);
};

class ColorTable{
  // A gif color table. Maps bytes to RGB-colors.
public:
  ColorTable(int entries){
    assert(entries >= 0);
    assert(entries <= 256);
    m_entries = entries;

    m_data = entries == 0 ? nullptr :
      new char[entries * 3];
  }

  ~ColorTable(){
    delete[] m_data;
  }

  Color Get(int index) const{
    assert(0 <= index);
    assert(index < m_entries);
    return Color(static_cast<unsigned char>(m_data[index * 3]),
      static_cast<unsigned char>(m_data[index * 3 + 1]),
      static_cast<unsigned char>(m_data[index * 3 + 2]));
  }

  char* RawData(){
    return m_data;
  }

  int RawLength(){
    return m_entries * 3;
  }

private:
  int m_entries;
  char* m_data;
};

enum class Disposal : int{
  MIN_VALUE = 0,

  NOT_SPECIFIED = 0,
  DO_NOT_DISPOSE = 1,
  RESTORE_TO_BACKGROUND_COLOR = 2,
  RESTORE_TO_PREVIOUS = 3,
  // Gif89a spec lists 4-7 as "to be defined"

  MAX_VALUE = 3
};

static Disposal to_disposal(std::underlying_type<Disposal>::type value){
  return to_enum(value, default_to(Disposal::NOT_SPECIFIED));
}

class DisposeInfo{
  // Keep track of how the old content should be restored before
  // drawing the next frame.
public:
  DisposeInfo()
    : disposal(Disposal::NOT_SPECIFIED)
  {}

  DisposeInfo(Disposal disposal, const IntRect& rect)
    : disposal(disposal),
      rect(rect)
  {}

  void Set(Disposal newDisposal, const IntRect& newRect){
    disposal = newDisposal;
    rect = newRect;
  }

  Disposal disposal;
  IntRect rect;
};

class GraphicControlExtension{
  // A parsed gif graphic control extension block
public:
  GraphicControlExtension(){
    disposal = Disposal::NOT_SPECIFIED;
    userInputFlag = 0;
    transparencyFlag = false;
    delayTime_cs = 0;
    transparencyIndex = 0;
  }

  DisposeInfo CreateDisposeInfo(const IntPoint& offset, const IntSize& size){
    return DisposeInfo(disposal, IntRect(offset, size));
  }

  auto GetDelay(){
    return Delay(delayTime_cs * 10);
  }

  Disposal disposal;
  int userInputFlag;
  bool transparencyFlag;
  int delayTime_cs;
  int transparencyIndex;
};

class ImageDescriptor{
  // A Parsed gif image descriptor
public:
  IntPoint offset;
  IntSize size;
  bool localColorTable;
  bool interlace;
  bool sort;
  unsigned int colorTableSize;
};

class IndexBuffer{
public:
  IndexBuffer(const IntSize& size, bool interlaced)
    : m_size(size),
      m_interlaced(interlaced)
  {
    m_data = new unsigned char[area(m_size)];
  }

  ~IndexBuffer(){
    delete[] m_data;
  }

  unsigned char& At(int x, int y){
    return m_data[x + y * m_size.w];
  }

  int Width() const{
    return m_size.w;
  }

  int Height() const{
    return m_size.h;
  }

  bool Interlaced() const{
    return m_interlaced;
  }
private:
  IndexBuffer(const IndexBuffer&);
  IntSize m_size;
  unsigned char* m_data;
  bool m_interlaced;
};

class LogicalScreenDescriptor{
  // Parsed gif logical screen descriptor
public:
  IntSize size;
  bool globalColorTableFlag;
  int colorResolution;
  bool sorted;
  int globalColorTableEntries;
  int backgroundColorIndex;
  int pixelAspectRatio;
};

class ParseData{
  // Keeps track of state during image data decoding
public:
  ParseData(){
    restBits = 0;
    restByte = 0;
    lastByte = 0;
    bufferPtr = buffer;
  }

  int restBits; // Remaining valid bits
  unsigned int restByte; // remaining bytes in this block
  unsigned int lastByte; // last byte read

  char buffer[256];
  char* bufferPtr;
};

enum class GifVer{GIF87a, GIF89a};

static GifVer read_gif_identifier(BinaryReader& in){
  char data[6];
  in.read(data, 6);
  if (!in.good()){
    throw LoadGifError("Premature EOF");
  }

  std::string versionStr(data, 6);
  if (versionStr == "GIF89a"){
    return GifVer::GIF89a;
  }
  else if (versionStr == "GIF87a"){
    return GifVer::GIF87a;
  }
  throw LoadGifError("Unknown GIF version");
}

static int to_int2(char b0, char b1){
  return static_cast<int>((uint(uchar(b1)) << 8) | (uint(uchar(b0))));
}

static LogicalScreenDescriptor read_logical_screen_descriptor(BinaryReader& in){
  LogicalScreenDescriptor lsd;
  char buf[7];
  in.read(buf, 7);
  if (!in.good()){
    throw LoadGifError("Premature EOF");
  }
  lsd.size.w = to_int2(buf[0], buf[1]);
  lsd.size.h = to_int2(buf[2], buf[3]);

  unsigned int packedFields = (unsigned int)((unsigned char)(buf[4]));
  lsd.globalColorTableFlag = ((packedFields & 128) == 128);
  lsd.colorResolution = ((packedFields & 0x70) >> 4) + 1;
  lsd.sorted = ((packedFields & 8) == 8);
  lsd.globalColorTableEntries = 1 << ((packedFields & 7) + 1);
  lsd.backgroundColorIndex = (int)((unsigned char)buf[5]);
  lsd.pixelAspectRatio = (int)buf[6];
  return lsd;
}

static void default_color_table(ColorTable& t){
  if (t.RawLength() >= 6){
    char* data = t.RawData();
    // Initialize the first two colors to black, white, ignore the
    // rest.
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;
    data[3] = (char)255u;
    data[4] = (char)255u;
    data[5] = (char)255u;
  }
}

static void read_color_table(BinaryReader& in, ColorTable& table){
  in.read(table.RawData(), table.RawLength());
  if (!in.good()){
    throw LoadGifError("Premature EOF in color table");
  }
}

static int stream_getc_i(BinaryReader& in){
  char c;
  in.read(&c, 1);
  if (!in.good()){
    throw LoadGifError("Premature EOF");
  }
  return (int)((unsigned char)c);
}

static int get_code(BinaryReader& in, const Alphabet& a, ParseData& pd){
  uint mask = (1 << a.bits) - 1;
  uint code = (pd.lastByte >> (8 - pd.restBits)) & mask;

  while (a.bits > pd.restBits){
    if (pd.restByte == 0){
      // No bytes left in current block, read the next block
      pd.restByte = static_cast<unsigned int>(stream_getc_i(in));
      assert(pd.restByte <= 256);

      if (pd.restByte == 0){
        // Zero length sub-block. FIXME: Should maybe allow.
        throw LoadGifError("Empty sub-block");
      }

      in.read(pd.buffer, resigned(pd.restByte));
      if (!in.good()){
        return a.endOfStreamCode;
      }
      pd.bufferPtr = pd.buffer;
    }

    pd.lastByte = (unsigned char)(*pd.bufferPtr++);
    mask = (1 << (a.bits - pd.restBits)) - 1;
    code = code + ((pd.lastByte & mask) << pd.restBits);
    pd.restByte--;

    pd.restBits += 8;
  }

  pd.restBits -= a.bits;
  return resigned(code);
}

bool find_interlaced_y(const IndexBuffer& img, int& y, int& pass){
  if (pass == 1 || pass == 2){
    y += 8;
  }
  else if (pass == 3){
    y += 4;
  }
  else if (pass == 4){
    y += 2;
  }
  // Loop until y is valid or the maximum number of passes is
  // reached.
  while (y >= img.Height()){
    pass += 1;
    if (pass == 2){
      y = 4;
    }
    else if (pass == 3){
      y = 2;
    }
    else if (pass == 4){
      y = 1;
    }
    else{
      y = 0;
      return false;
    }
  }
  return true;
}

static int flush_stack(Alphabet& a,
  IndexBuffer& img,
  int& x,
  int& y,
  int& pass,
  int code)
{
  while (!a.Empty()){
    img.At(x, y) = (unsigned char)a.Pop();
    if (++x >= img.Width()){
      x = 0;
      if (img.Interlaced()){
        bool found = find_interlaced_y(img, y, pass);
        if (!found){
          break;
        }
      }
      else{
        y++;
        if (y >= img.Height()){
          return a.endOfStreamCode;
        }
      }
    }
  }
  a.ResetStack();
  return code;
}

static void read_image_data(BinaryReader& in, IndexBuffer& img){
  // Read the initial code size
  char b = 0;
  in.read(&b,1);
  if (!in.good()){
    throw LoadGifError("Premature EOF in image data");
  }
  if (b == 0){
    throw LoadGifError("Invalid initial byte in image data");
  }

  Alphabet a((unsigned char)b);

  int lastCode = -1;
  int prevCode = -1;

  // For interlaced images
  int pass = 1;

  int x = 0;
  int y = 0;

  ParseData pd;
  for (;;){
    int code = get_code(in, a, pd);
    const int readCode = code;
    if (code == a.endOfStreamCode){
      return;
    }

    if (code == a.clearCode){
      a.Reset();
      lastCode = -1;
      prevCode = -1;
      continue;
    }

    if (code >= a.freeIndex){
      // Unknown code, common special case, not sure what it means
      code = lastCode;

      // Add the first character
      a.Push(prevCode);
    }

    // Build the string for this code in the stack
    code = a.BuildString(code);

    // Set last code at the top of the stack
    a.Set(code);
    prevCode = code;

    if (lastCode != -1){
      a.NewEntry(code, lastCode);
    }

    // Flush the stack into the index buffer
    code = flush_stack(a, img, x, y, pass, code);
    lastCode = readCode;
    if (code == a.endOfStreamCode){
      return;
    }
  }
}

static ImageDescriptor read_image_descriptor(BinaryReader& in){
  char buf[9];
  in.read(buf, 9);
  if (!in.good()){
    throw LoadGifError("Premature EOF in image descriptor");
  }

  ImageDescriptor descriptor;
  descriptor.offset.x = to_int2(buf[0], buf[1]);
  descriptor.offset.y = to_int2(buf[2], buf[3]);
  descriptor.size.w = to_int2(buf[4], buf[5]);
  descriptor.size.h = to_int2(buf[6], buf[7]);
  unsigned int packedFields = ((unsigned int)(unsigned char)buf[8]);

  descriptor.localColorTable = (packedFields & 128) == 128;
  descriptor.interlace = (packedFields & 64) == 64;
  descriptor.sort = (packedFields & 32) == 32;
  descriptor.colorTableSize = 1 << ((packedFields & 0x7) + 1);
  return descriptor;
}

static GraphicControlExtension read_graphic_control_extension(BinaryReader& in){
  char data[6];
  in.read(data, 6);
  if (!in.good()){
    throw LoadGifError("Premature EOF within graphic control extension");
  }
  if ((unsigned char)data[0] != 4){
    throw LoadGifError("Unexpected size of graphic control extension");
  }

  unsigned char packed = static_cast<unsigned char>(data[1]);
  GraphicControlExtension ext;
  ext.disposal = to_disposal((((packed & 0x1c) >> 2)));
  ext.userInputFlag = ((packed & 0x2) == 0x2);
  ext.transparencyFlag = ((packed & 0x1) == 0x1);
  ext.delayTime_cs = to_int2(data[2], data[3]);
  ext.transparencyIndex = (int)((unsigned char)data[4]);
  if (data[5] != 0){
    throw LoadGifError("Invalid block terminator in graphic control extension");
  }
  return ext;
}

static void read_application_extension(BinaryReader& in){
  // Fixme: Maybe implement
  char entry = 0;
  do {
    in.read(&entry, 1);
    if (!in.good()){
      throw LoadGifError("Premature EOF within extension block");
    }
  } while (entry != 0);
}

  static void read_away_unknown_extension(BinaryReader& in, unsigned char type){
  char entry;
  std::streampos extensionStart = in.tellg();
  do {
    in.read(&entry, 1);
    if (!in.good()){
      std::stringstream ss;
      ss << "Premature EOF within extension block of type: " <<
        std::hex << static_cast<int>(type) << std::endl
         << "Starting at " << extensionStart;
      throw LoadGifError(ss.str().c_str());
    }
  } while (entry != 0);
}

static Bitmap get_bg(ImageProps& props,
  const LogicalScreenDescriptor& lsd,
  const DisposeInfo& disposeInfo)
{
  if (props.GetNumFrames() == 0){
    return Bitmap(lsd.size, color_transparent_white);
  }

  Disposal d = disposeInfo.disposal;
  switch (d){
  case Disposal::RESTORE_TO_BACKGROUND_COLOR:
    {
      return props.GetFrame(props.GetNumFrames() - 1).GetBackground().Visit(
        [&](const Bitmap& bmp){
          Bitmap bg(bmp);
          fill_rect_color(bg, disposeInfo.rect,
            color_transparent_white);
          return bg;
        },
        [](const ColorSpan&) -> Bitmap {
          assert(false);
          return Bitmap();
        });
    }
  case Disposal::RESTORE_TO_PREVIOUS:
    return props.GetNumFrames() > 1 ?
      props.GetFrame(props.GetNumFrames() - 2).GetBackground().Expect<Bitmap>() :
      Bitmap(lsd.size, color_transparent_white);

  case Disposal::NOT_SPECIFIED:
  case Disposal::DO_NOT_DISPOSE:
  default:
    return props.GetFrame(props.GetNumFrames() - 1)
      .GetBackground().Expect<Bitmap>();
  }
}

static bool read_block(BinaryReader& in,
  ImageProps& props,
  LogicalScreenDescriptor& lsd,
  ColorTable& globalColorTable,
  GraphicControlExtension& gce,
  DisposeInfo& disposeInfo)
{
  char identifier;
  in.read(&identifier, 1);
  if (!in.good()){
    return false;
  }

  if (identifier == 0x21){
    in.read(&identifier, 1);
    if (!in.good()){
      throw LoadGifError("Unexpected end of file after extension identifier");
    }
    if ((unsigned char)identifier == 0xf9){
      gce = read_graphic_control_extension(in);
    }
    else if ((unsigned char)identifier == 0xff){
      read_application_extension(in);
    }
    else{
      read_away_unknown_extension(in, (unsigned char)identifier);
    }
  }
  else if (identifier == 0x2c){
    ImageDescriptor descr = read_image_descriptor(in);
    // Use a local color table if available, otherwise
    // use the global color table
    ColorTable localColorTable(descr.localColorTable ?
      resigned(descr.colorTableSize) : 0);
    if (descr.localColorTable){
      read_color_table(in, localColorTable);
    }
    const ColorTable& colorTable = descr.localColorTable ?
      localColorTable : globalColorTable;

    IndexBuffer img(descr.size, descr.interlace);
    read_image_data(in, img);

    Bitmap bmp(descr.size);
    for (int y = 0; y != img.Height(); y++){
      for (int x = 0; x != img.Width(); x++){
        unsigned char index = img.At(x,y);
        if (!gce.transparencyFlag || index != gce.transparencyIndex){
          put_pixel_raw(bmp, x, y, colorTable.Get(index));
        }
      }
    }

    Bitmap bg = get_bg(props, lsd, disposeInfo);

    blend(offsat(bmp, descr.offset), onto(bg));
    props.AddFrame(std::move(bg), FrameInfo(gce.GetDelay()));

    disposeInfo = gce.CreateDisposeInfo(descr.offset, descr.size);
    gce = GraphicControlExtension();
  }
  return true;
}

void read_gif(const FilePath& filePath, ImageProps& props){
  try{
    BinaryReader f(filePath);
    if (!f.good()){
      throw LoadGifError("Could not open " + filePath.Str() + "for reading.");
    }

    GifVer gifVersion = read_gif_identifier(f);
    LogicalScreenDescriptor lsd = read_logical_screen_descriptor(f);

    int entries = lsd.globalColorTableFlag ? lsd.globalColorTableEntries : 256;
    ColorTable globalColorTable(entries);
    if (lsd.globalColorTableFlag){
      read_color_table(f, globalColorTable);
    }
    else{
      default_color_table(globalColorTable);
    }
    bool ok = true;
    GraphicControlExtension gce;
    DisposeInfo disposeInfo;
    while (ok){
      try{
        ok = read_block(f, props, lsd, globalColorTable, gce, disposeInfo);
      }
      catch (const LoadGifError& error){
        if (props.GetNumFrames() == 0){
          throw;
        }
        else{
          props.AddWarning(error.GetString());
          ok = false;
        }
      }
    }

    if (props.GetNumFrames() > 1 && gifVersion == GifVer::GIF87a){
      props.AddWarning(utf8_string(
        "GIF specified as GIF87a, but contains multiple frames."));
    }
  }
  catch(const LoadGifError& e){
    props.SetError(e.GetString());
  }
}

} // namespace
