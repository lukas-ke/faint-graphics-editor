// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#include <cassert>
#include <map>
#include <vector>
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color-list.hh"
#include "formats/bmp/bmp-errors.hh"
#include "formats/bmp/file-cur.hh"
#include "formats/bmp/file-ico.hh"
#include "formats/bmp/serialize-bmp-pixel-data.hh"
#include "formats/bmp/serialize-bmp-types.hh"
#include "geo/int-point.hh"
#include "geo/limits.hh"
#include "text/formatting.hh"
#include "util-wx/encode-bitmap.hh"
#include "util-wx/stream.hh"
#include "util/index-iter.hh"
#include "util/iter.hh"
#include "util/make-vector.hh"

namespace faint{

static uint to_uint(char b){
  return uint(uchar(b));
}

static bool is_png(char* data){
  return to_uint(data[0]) == 0x89 &&
    to_uint(data[1]) == 0x50 &&
    to_uint(data[2]) == 0x4e &&
    to_uint(data[3]) == 0x47 &&
    to_uint(data[4]) == 0x0d &&
    to_uint(data[5]) == 0x0a &&
    to_uint(data[6]) == 0x1a &&
    to_uint(data[7]) == 0x0a;
}

static bool peek_png_signature(BinaryReader& in, IconType type, Index i){
  std::streampos oldPos = in.tellg();
  char signature[8];
  in.read(signature, 8);
  if (!in.good() || in.eof()){
    throw ReadBmpError(error_image(type, i));
  }
  in.seekg(oldPos);
  return is_png(signature);
}

static void test_bitmap_header(IconType type,
  Index iconNum,
  const BitmapInfoHeader& h)
{
  if (invalid_header_length(h.headerLen)){
    throw ReadBmpError(error_truncated_bmp_header(type, iconNum, h.headerLen));
  }
  if (h.compression != Compression::BI_RGB){
    throw ReadBmpError(error_compression(type, iconNum, h.compression));
  }
  if (h.colorPlanes != 1){
    throw ReadBmpError(error_color_planes(type, iconNum, h.colorPlanes));
  }
}

static void test_icon_dir_common(const IconDir& dir){
  if (dir.reserved != 0){
    throw ReadBmpError(error_dir_reserved(dir.imageType, dir.reserved));
  }
}

static auto read_icon_dir_entries(BinaryReader& in, IconType type, Index numIcons){
  assert(numIcons >= 0);
  return make_vector(up_to(numIcons), [&](const auto& i){
    auto entry = read_struct_or_throw_ico<IconDirEntry>(in, type, option(i));
    if (!in.good()){
      throw ReadBmpError(error_premature_eof_ico(type, "ICONDIRENTRY", option(i)));
    }
    return entry;
  });
}

static Bitmap masked(const Bitmap& bmp, const AlphaMap& mask){
  Bitmap out(bmp);
  auto size = mask.GetSize();
  for (int y = 0; y!= size.h; y++){
    for (int x = 0; x != size.w; x++){
      if (mask.Get(x,y) == 1){
        put_pixel(out, {x,y}, color_transparent_white);
      }
    }
  }
  return out;
}


auto get_read_pixeldata_func(const Index num, int bitsPerPixel)
  -> std::function<Optional<Bitmap>(BinaryReader&, const BmpSizeAndOrder&)>
{
  // Returns a function for reading pixeldata of the specified
  // bits-per-pixel, or throws ReadBmpError if the bitsPerPixel is
  // unsupported.

  if (bitsPerPixel == 1){

    return [](BinaryReader& in, const BmpSizeAndOrder& size) -> Optional<Bitmap>{
      auto colorTable = read_color_table(in, 2);
      if (colorTable.NotSet()){
        return {};
      }

      auto xorMask = read_1bipp_BI_RGB(in, size);
      if (xorMask.NotSet()){
        return {};
      }

      auto andMask = read_1bipp_BI_RGB(in, size);
      if (andMask.NotSet()){
        return {};
      }

      auto bmp = bitmap_from_indexed_colors(xorMask.Get(), colorTable.Get());
      return masked(bmp, andMask.Get());
    };
  }
  else if (bitsPerPixel == 4){

    return [](BinaryReader& in, const BmpSizeAndOrder& size) -> Optional<Bitmap>{
      auto colorTable = read_color_table(in, 16);
      if (colorTable.NotSet()){
        return {};
      }

      auto xorMask = read_4bipp_BI_RGB(in, size);
      if (xorMask.NotSet()){
        return {};
      }

      auto andMask = read_1bipp_BI_RGB(in, size);
      if (andMask.NotSet()){
        return {};
      }

      auto bmp = bitmap_from_indexed_colors(xorMask.Get(), colorTable.Get());
      return masked(bmp, andMask.Get());
    };
  }
  else if (bitsPerPixel == 24){
    return [](BinaryReader& in, const BmpSizeAndOrder& bmpSize) -> Optional<Bitmap>{
      auto xorMask = read_24bipp_BI_RGB(in, bmpSize);
      if (xorMask.NotSet()){
        return {}; // TODO: Error?
      }

      auto andMask = read_1bipp_BI_RGB(in, bmpSize);
      if (andMask.NotSet()){
        return {}; // TODO: Error?
      }

      if (andMask.Get().GetSize() != xorMask.Get().GetSize()){
        return {}; // TODO: Error? Also maybe unnecessary check
      }

      return masked(xorMask.Get(), andMask.Get());
    };
  }
  else if (bitsPerPixel == 32){

    return [](BinaryReader& in, const BmpSizeAndOrder& bitmapSize) -> Optional<Bitmap>{
      const int bypp = 4;
      // The size from the bmp-header. May have larger height than the size
      // in the IconDirEntry.
      int bufLen = area(bitmapSize.size) * bypp;
      std::vector<char> pixelData(bufLen);
      in.read(pixelData.data(), bufLen);
      if (!in.good()){
        return {};
      }

      // The size from the IconDirEntry is the exact amount of pixels
      // this image should have.
      const IntSize& sz(bitmapSize.size);
      Bitmap bmp(sz);
      for (int y = 0; y != sz.h; y++){
        for (int x = 0; x != sz.w; x++){
          uint b = to_uint(pixelData[y * sz.w * bypp + x * bypp]);
          uint g = to_uint(pixelData[y * sz.w * bypp + x * bypp + 1]);
          uint r = to_uint(pixelData[y * sz.w * bypp + x * bypp + 2]);
          uint a = to_uint(pixelData[y * sz.w * bypp + x * bypp + 3]);

          const auto yDst = bitmapSize.topDown ? y : bmp.m_h - y - 1;
          put_pixel_raw(bmp, x, yDst, color_from_uints(r,g,b,a));
        }
      }
      return std::move(bmp);
    };
  }
  else{
    throw ReadBmpError(error_bits_per_pixel(num, bitsPerPixel));
  }
}

static OrError<Bitmap> ico_read_png(BinaryReader& in, int len){
  assert(len > 0);
  std::vector<char> v(len);
  in.read(v.data(), len);
  if (!in.good()){
    return {"Failed reading png stream"};
  }

  return from_png(v.data(), to_size_t(len));
}

class Ico{
public:
  using ResultType = bmp_vec;
  static auto error_bmp_data(Index i){
    return error_bmp_data_ico(i);
  }

  static void add(ResultType& v, Bitmap&& bmp, const IconDirEntry&){
    v.emplace_back(std::move(bmp));
  }

  static void test_icon_dir(const IconDir& dir){
    test_icon_dir_common(dir);
    if (dir.imageType == IconType::CUR){
      throw ReadBmpError(error_icon_is_cursor());
    }
    if (dir.imageType != IconType::ICO){
      throw ReadBmpError(error_unknown_image_type(dir.imageType));
    }
  }

  static IconType type(){
    return IconType::CUR;
  }
};

class Cur{
public:
  using ResultType = cur_vec;
  static auto error_bmp_data(Index i){
    return error_bmp_data_cur(i);
  }

  static void add(ResultType& v, Bitmap&& bmp, const IconDirEntry& iconDirEntry){
    v.emplace_back(std::move(bmp), get_hot_spot(iconDirEntry));
  }

  static void test_icon_dir(const IconDir& dir){
    test_icon_dir_common(dir);
    if (dir.imageType == IconType::ICO){
      throw ReadBmpError(error_cursor_is_icon());
    }
    if (dir.imageType != IconType::CUR){
      throw ReadBmpError(error_unknown_image_type(dir.imageType));
    }
    if (dir.imageCount == 0){
      throw ReadBmpError(error_no_images());
    }
  }

  static IconType type(){
    return IconType::CUR;
  }
};

// Reads the specified BmpType (either Cur or Ico).
// Throws ReadBmpError on failure, otherwise returns a vector of cursors
// or icons (as determined by BmpType::ResultType).
template<typename BmpType>
typename BmpType::ResultType read_or_throw(const FilePath& filePath){
  BinaryReader in(filePath);
  if (!in.good()){
    throw ReadBmpError(error_open_file_read(filePath));
  }

  auto iconDir = read_struct_or_throw_ico<IconDir>(in,
    BmpType::type(),
    no_option());

  if (!in.good()){
    throw ReadBmpError(error_premature_eof_ico(BmpType::type(),
      struct_name<IconDir>(),
      no_option()));
  }

  BmpType::test_icon_dir(iconDir);
  auto iconEntries(read_icon_dir_entries(in,
    BmpType::type(),
    Index(iconDir.imageCount)));

  if (!in.good()){
    throw ReadBmpError("ICONDIRENTRY");
  }

  typename BmpType::ResultType bitmaps;
  for (auto i : up_to(iconEntries.size())){
    IconDirEntry& iconDirEntry = iconEntries[to_size_t(i)];
    in.seekg(iconDirEntry.offset);
    if (!in.good() || in.eof()){
      throw ReadBmpError(error_read_to_offset(i, iconDirEntry.offset));
    }

    if (peek_png_signature(in, BmpType::type(), i)){
      ico_read_png(in, iconDirEntry.bytes).Visit(
        [&](Bitmap bmp){
          BmpType::add(bitmaps, std::move(bmp), iconDirEntry);
        },
        [i](const utf8_string& errorInfo ){
          throw ReadBmpError(endline_sep(error_truncated_png_data(Index(i)),
            errorInfo));
        });
    }
    else{
      const auto on_error_image = [i](){
        return BmpType::error_bmp_data(i);
      };

      auto bmpHeader = read_struct_or_throw_ico<BitmapInfoHeader>(in,
        BmpType::type(), Index(i));
      test_bitmap_header(BmpType::type(), Index(i), bmpHeader);
      const IntSize imageSize(get_size(iconDirEntry));
      const BmpSizeAndOrder bmpSize = {imageSize, false}; // Fixme: Assumption, icons never top down

      auto read_pixeldata = get_read_pixeldata_func(i, bmpHeader.bitsPerPixel);
      auto bmp = or_throw(read_pixeldata(in, bmpSize), on_error_image);
      BmpType::add(bitmaps, std::move(bmp), iconDirEntry);
    }
  }
  return bitmaps;
}

OrError<bmp_vec> read_ico(const FilePath& filePath){
  try {
    return read_or_throw<Ico>(filePath);
  }
  catch (const ReadBmpError& e){
    return e.message;
  }
}

OrError<cur_vec> read_cur(const FilePath& filePath){
  try {
    return read_or_throw<Cur>(filePath);
  }
  catch (const ReadBmpError& error){
    return error.message;
  }
}

static std::vector<BitmapInfoHeader> create_bitmap_headers(const ico_vec& icons,
  const std::map<size_t, std::string>& pngData)
{
  return make_vector(enumerate(icons), [&](const auto& icon){
    const auto& [bmp, compression] = icon.item;
    const auto i = icon.num;

    if (compression == IcoCompression::PNG){
      return create_bitmap_info_header_png(bmp.GetSize(),
        pngData.at(i).size(),
        default_DPI());
    }
    else{
      return create_bitmap_info_header_32bipp(bmp.GetSize(),
        default_DPI(),
        true);
    }
  });
}

static std::vector<BitmapInfoHeader> create_bitmap_headers(const cur_vec& cursors){
  return make_vector(cursors,
    [](const auto& c){
      return create_bitmap_info_header_32bipp(c.first.GetSize(),
        default_DPI(),
        true);
    });
}

static IconDir create_icon_dir_icon(const ico_vec& icons){
  IconDir iconDir;
  iconDir.reserved = 0;
  iconDir.imageType = IconType::ICO;
  iconDir.imageCount = convert(icons.size());
  return iconDir;
}

static IconDir create_icon_dir_cursor(const cur_vec& cursors){
  IconDir iconDir;
  iconDir.reserved = 0;
  iconDir.imageType = IconType::CUR;
  iconDir.imageCount = convert(cursors.size());
  return iconDir;
}

static std::vector<IconDirEntry> create_icon_dir_entries(const ico_vec& icons,
  const std::map<size_t, std::string>& pngData)
{
  std::vector<IconDirEntry> v;

  auto offset = struct_lengths<IconDir>() +
    icons.size() * struct_lengths<IconDirEntry>();

  for (size_t i = 0; i != icons.size(); i++){
    const auto& [bmp, compression] = icons[i];

    IconDirEntry entry;
    set_size(entry, bmp.GetSize());
    entry.reserved = 0;
    entry.colorCount = 0; // 0 when bitsPerPixel >= 8
    entry.colorPlanes = 1;
    entry.bitsPerPixel = 32;

    if (compression == IcoCompression::PNG){
      const auto& pngStr = pngData.at(i);
      const auto bytes = pngStr.size();
      entry.bytes = convert(bytes);

      const size_t lengthOfHeaders = struct_lengths<
        BitmapInfoHeader, BitmapFileHeader>();

      // Offset to png-data, i.e. past both headers.
      entry.offset = convert(offset + lengthOfHeaders);
      offset += convert(entry.bytes + lengthOfHeaders);
    }
    else if (compression == IcoCompression::BMP){
      entry.bytes = convert(area(bmp.GetSize()) * 4 +
        and_map_bytes(bmp.GetSize()) +
        struct_lengths<BitmapInfoHeader>());
      entry.offset = convert(offset);
      offset += entry.bytes;
    }
    else{
      assert(false);
    }

    v.push_back(entry);
  }
  return v;
}

static std::vector<IconDirEntry> create_cursor_dir_entries(const cur_vec& cursors){
  auto offset = struct_lengths<IconDir>() +
    cursors.size() * struct_lengths<IconDirEntry>();

  return make_vector(cursors, [offset](const auto& obj) mutable {
    const auto& [bmp, hotspot] = obj;

    IconDirEntry entry;
    set_size(entry, bmp.GetSize());
    entry.reserved = 0;
    entry.colorCount = 0; // 0 when bitsPerPixel >= 8
    set_hot_spot(entry, hotspot);
    entry.bytes = convert(area(bmp.GetSize()) * 4 +
      and_map_bytes(bmp.GetSize()) +
      struct_lengths<BitmapInfoHeader>());
    entry.offset = convert(offset);
    offset += entry.bytes;
    return entry;
  });
}

BitmapFileHeader create_bitmap_file_header_png(size_t encodedSize){
  const auto headerLengths = struct_lengths<BitmapInfoHeader, BitmapFileHeader>();

  BitmapFileHeader h;
  h.fileType = BITMAP_SIGNATURE;
  h.fileLength = convert(headerLengths + encodedSize);
  h.reserved1 = 0;
  h.reserved2 = 0;
  h.dataOffset = headerLengths;
  return h;
}

SaveResult write_ico(const FilePath& filePath, const ico_vec& icons){
  assert(!icons.empty());
  if (icons.size() > UINT16_MAX){
    return SaveResult::SaveFailed(error_ico_too_many_images(icons.size()));
  }

  for (const auto& p : icons){
    const auto& bmp = p.first;
    if (bmp.m_w > 256 || bmp.m_h > 256){
      return SaveResult::SaveFailed(utf8_string(
        "Maximum size for icons is 256x256"));
    }
  }

  std::map<size_t, std::string> pngData;
  for (size_t i = 0; i != icons.size(); i++){
    const auto& [bmp, compression] = icons[i];
    if (compression == IcoCompression::PNG){
      pngData[i] = to_png_string(bmp);
    }
  }

  const auto iconDir = create_icon_dir_icon(icons);
  const auto iconDirEntries = create_icon_dir_entries(icons, pngData);
  const auto bmpHeaders = create_bitmap_headers(icons, pngData);

  BinaryWriter out(filePath);
  if (!out.good()){
    return SaveResult::SaveFailed(error_open_file_write(filePath));
  }

  write_struct(out, iconDir);
  for (const auto& iconDirEntry : iconDirEntries){
    write_struct(out, iconDirEntry);
  }

  assert(icons.size() == bmpHeaders.size());
  for (size_t i = 0; i != icons.size(); i++){
    const auto& [bmp, compression] = icons[i];
    if (compression == IcoCompression::PNG){
      const auto& pngStr = pngData.at(i);
      write_struct(out, create_bitmap_file_header_png(pngStr.size()));
      write_struct(out, bmpHeaders[i]);
      out.write(pngStr.c_str(), pngStr.size());
    }
    else{
      write_struct(out, bmpHeaders[i]);
      write_32bipp_BI_RGB_ICO(out, bmp);
    }
  }
  return SaveResult::SaveSuccessful();
}

SaveResult write_cur(const FilePath& filePath, const cur_vec& cursors){
  assert(!cursors.empty());
  if (cursors.size() > UINT16_MAX){
    return SaveResult::SaveFailed(error_ico_too_many_images(cursors.size()));
  }

  for (const auto& cur : cursors){
    const auto& bmp(cur.first);
    if (bmp.m_w > 256 || bmp.m_h > 256){
      return SaveResult::SaveFailed(utf8_string(
        "The maximum size for cursors is 256x256 pixels."));
    }
  }
  IconDir iconDir = create_icon_dir_cursor(cursors);
  auto iconDirEntries = create_cursor_dir_entries(cursors);
  auto bmpHeaders = create_bitmap_headers(cursors);

  BinaryWriter out(filePath);
  if (!out.good()){
    return SaveResult::SaveFailed(error_open_file_write(filePath));
  }

  write_struct(out, iconDir);
  for (const auto& iconDirEntry : iconDirEntries){
    write_struct(out, iconDirEntry);
  }

  assert(cursors.size() == bmpHeaders.size());
  for (size_t i = 0; i != cursors.size(); i++){
    write_struct(out, bmpHeaders[i]);
    write_32bipp_BI_RGB_ICO(out, cursors[i].first);
  }
  return SaveResult::SaveSuccessful();
}

} // namespace
