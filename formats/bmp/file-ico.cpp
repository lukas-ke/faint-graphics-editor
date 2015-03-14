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
#include "text/formatting.hh"
#include "util-wx/encode-bitmap.hh"
#include "util-wx/stream.hh"
#include "util/iter.hh"

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

static bool peek_png_signature(BinaryReader& in, const size_t i){
  std::streampos oldPos = in.tellg();
  char signature[8];
  in.read(signature, 8);
  if (!in.good() || in.eof()){
    throw ReadBmpError(error_image(i));
  }
  in.seekg(oldPos);
  return is_png(signature);
}

static auto create_bitmap_info_header_png(const Bitmap& bmp, size_t rawDataSize){
  BitmapInfoHeader h;
  h.headerLen = struct_lengths<BitmapInfoHeader>();
  IntSize bmpSize = bmp.GetSize();
  h.width = bmpSize.w;
  h.height = bmpSize.h;
  h.colorPlanes = 1;
  h.bpp = 0;
  h.compression = Compression::BI_PNG;
  h.rawDataSize = static_cast<uint32_t>(rawDataSize); // Fixme: Check cast
  h.horizontalResolution = 1; // Fixme: OK?
  h.verticalResolution = 1; // Fixme: OK?
  h.paletteColors = 0;
  h.importantColors = 0;
  return h;
}

static void test_bitmap_header(size_t iconNum, const BitmapInfoHeader& h){
  if (invalid_header_length(h.headerLen)){
    throw ReadBmpError(error_truncated_bmp_header(iconNum, h.headerLen));
  }
  if (h.compression != Compression::BI_RGB){
    throw ReadBmpError(error_compression(iconNum, h.compression));
  }
  if (h.colorPlanes != 1){
    throw ReadBmpError(error_color_planes(iconNum, h.colorPlanes));
  }
}

static void test_icon_dir_common(const IconDir& dir){
  if (dir.reserved != 0){
    throw ReadBmpError(error_dir_reserved(dir.reserved));
  }
}

static void test_icon_dir_ico(const IconDir& dir){
  test_icon_dir_common(dir);
  if (dir.imageType == IconType::CUR){
    throw ReadBmpError(error_icon_is_cursor());
  }
  if (dir.imageType != IconType::ICO){
    throw ReadBmpError(error_unknown_image_type(dir.imageType));
  }
}

static void test_icon_dir_cur(const IconDir& dir){
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

static auto read_icon_dir_entries(BinaryReader& in, int numIcons){
  assert(numIcons >= 0);
  std::vector<IconDirEntry> v;
  for (int i = 0; i != numIcons; i++){
    auto entry = read_struct_or_throw<IconDirEntry>(in);
    if (!in.good()){
      throw ReadBmpError(error_premature_eof("ICONDIRENTRY"));
    }
    v.push_back(entry);
  }
  return v;
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

static Optional<Bitmap> read_1bpp_ico(BinaryReader& in, const IntSize& size){
  auto colorTable = read_color_table(in, 2);
  if (colorTable.NotSet()){
    return {};
  }

  auto xorMask = read_1bpp_BI_RGB(in, size);
  if (xorMask.NotSet()){
    return {};
  }

  auto andMask = read_1bpp_BI_RGB(in, size);
  if (andMask.NotSet()){
    return {};
  }

  auto bmp = bitmap_from_indexed_colors(xorMask.Get(), colorTable.Get());
  return masked(bmp, andMask.Get());
}

static Optional<Bitmap> read_4bpp_ico(BinaryReader& in, const IntSize& size){
  auto colorTable = read_color_table(in, 16);
  if (colorTable.NotSet()){
    return {};
  }

  auto xorMask = read_4bpp_BI_RGB(in, size);
  if (xorMask.NotSet()){
    return {};
  }

  auto andMask = read_1bpp_BI_RGB(in, size);
  if (andMask.NotSet()){
    return {};
  }

  auto bmp = bitmap_from_indexed_colors(xorMask.Get(), colorTable.Get());
  return masked(bmp, andMask.Get());
}

static Optional<Bitmap> ico_read_32bpp_BI_RGB(BinaryReader& in,
  const IntSize& bitmapSize)
{
  int bypp = 4;
  // The size from the bmp-header. May have larger height than the size
  // in the IconDirEntry. (Fixme: Why?)
  int bufLen = area(bitmapSize) * bypp;
  std::vector<char> pixelData(bufLen);
  in.read(pixelData.data(), bufLen);
  if (!in.good()){
    return {};
  }

  // The size from the IconDirEntry is the exact amount of pixels
  // this image should have.
  const IntSize& sz(bitmapSize);
  Bitmap bmp(sz);
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      uint b = to_uint(pixelData[y * sz.w * bypp + x * bypp]);
      uint g = to_uint(pixelData[y * sz.w * bypp + x * bypp + 1]);
      uint r = to_uint(pixelData[y * sz.w * bypp + x * bypp + 2]);
      uint a = to_uint(pixelData[y * sz.w * bypp + x * bypp + 3]);
      put_pixel_raw(bmp, x, bmp.m_h - y - 1, color_from_uints(r,g,b,a));
    }
  }
  return std::move(bmp);
}

static Optional<Bitmap> ico_read_png(BinaryReader& in, int len){
  assert(len > 0);
  std::vector<char> v(len);
  in.read(v.data(), len);
  if (!in.good()){
    return {};
  }

  return from_png(v.data(), to_size_t(len));
}

bmp_vec read_ico_or_throw(const FilePath& filePath){
  BinaryReader in(filePath);
  std::vector<Bitmap> bitmaps;
  if (!in.good()){
    throw ReadBmpError(error_open_file_read(filePath));
  }

  auto iconDir = read_struct_or_throw<IconDir>(in);
  if (!in.good()){
    throw ReadBmpError(error_premature_eof("ICONDIR"));
  }
  test_icon_dir_ico(iconDir);
  auto iconEntries(read_icon_dir_entries(in, iconDir.imageCount));

  for (size_t i = 0; i != iconEntries.size(); i++){
    IconDirEntry& iconDirEntry = iconEntries[i];
    in.seekg(iconDirEntry.offset);
    if (!in.good() || in.eof()){
      throw ReadBmpError(error_read_to_offset(i, iconDirEntry.offset));
    }

    if (peek_png_signature(in, i)){
      auto bmp = or_throw(ico_read_png(in, iconDirEntry.bytes),
        [i](){return error_truncated_png_data(Index(i));});
      bitmaps.emplace_back(std::move(bmp));
    }
    else {
      const auto onErrorImage([i](){return error_image(i);});
      const IntSize imageSize(get_size(iconDirEntry));

      auto bmpHeader = read_struct_or_throw<BitmapInfoHeader>(in);
      test_bitmap_header(i, bmpHeader);

      if (bmpHeader.bpp == 1){
        auto bmp = or_throw(read_1bpp_ico(in, imageSize), onErrorImage);
        bitmaps.emplace_back(bmp);
      }
      else if (bmpHeader.bpp == 4){
        auto bmp = or_throw(read_4bpp_ico(in, imageSize), onErrorImage);
        bitmaps.emplace_back(bmp);
      }
      else if (bmpHeader.bpp == 8){
        bitmaps.emplace_back(IntSize(10,10), color_white); // Fixme
      }
      else if (bmpHeader.bpp == 32){
        auto bmp = or_throw(ico_read_32bpp_BI_RGB(in, imageSize), onErrorImage);
        bitmaps.emplace_back(std::move(bmp));
      }
      else {
        throw ReadBmpError(error_bpp(i, bmpHeader.bpp));
      }
    }
  }
  return bitmaps;
}

OrError<bmp_vec> read_ico(const FilePath& filePath){
  try {
    return read_ico_or_throw(filePath);
  }
  catch (const ReadBmpError& e){
    return e.message;
  }
}

static cur_vec read_cur_or_throw(const FilePath& filePath){
  // Fixme: Mostly duplicates read_ico_or_throw

  BinaryReader in(filePath);
  if (!in.good()){
    throw ReadBmpError(error_open_file_read(filePath));
  }

  auto cursorDir = read_struct_or_throw<IconDir>(in);
  if (!in.good()){
    throw ReadBmpError(error_premature_eof("ICONDIR"));
  }

  test_icon_dir_cur(cursorDir);
  auto iconDirEntries = read_icon_dir_entries(in, cursorDir.imageCount);
  if (!in.good()){
    throw ReadBmpError(error_premature_eof("ICONDIRENTRY"));
  }

  cur_vec cursors;
  for (size_t i = 0; i != iconDirEntries.size(); i++){
    IconDirEntry& iconDirEntry = iconDirEntries[i];
    in.seekg(iconDirEntry.offset);
    if (!in.good() || in.eof()){
      throw ReadBmpError(error_read_to_offset(i, iconDirEntry.offset));
    }

    bool pngCompressed = peek_png_signature(in, i);

    auto hotSpot(get_hot_spot(iconDirEntry));
    if (pngCompressed){
      auto bmp = or_throw(ico_read_png(in, iconDirEntry.bytes),
        [i](){return error_truncated_png_data(Index(i));});
      cursors.emplace_back(std::make_pair(std::move(bmp), hotSpot));
    }
    else {
      auto bmpHeader = read_struct_or_throw<BitmapInfoHeader>(in);
      test_bitmap_header(i, bmpHeader);
      if (bmpHeader.bpp == 1){
        auto bmp = or_throw(read_1bpp_ico(in, get_size(iconDirEntry)),
          [i](){return error_bmp_data(i);});
        cursors.emplace_back(bmp, hotSpot);
      }
      else if (bmpHeader.bpp == 4){
        auto bmp = or_throw(read_4bpp_ico(in, get_size(iconDirEntry)),
          [i](){return error_bmp_data(i);});
        cursors.emplace_back(bmp, hotSpot);
      }
      else if (bmpHeader.bpp == 32){
        auto bmp = or_throw(ico_read_32bpp_BI_RGB(in, get_size(iconDirEntry)),
          [i](){return error_bmp_data(i);});
        cursors.emplace_back(std::make_pair(std::move(bmp), hotSpot));
      }
      else {
        throw ReadBmpError(error_bpp(i, bmpHeader.bpp));
      }
    }
  }
  return cursors;
}

OrError<cur_vec> read_cur(const FilePath& filePath){
  try {
    return read_cur_or_throw(filePath);
  }
  catch (const ReadBmpError& error){
    return error.message;
  }
}

static std::vector<BitmapInfoHeader> create_bitmap_headers(const ico_vec& bitmaps,
  const std::map<size_t, std::string>& pngData)
{
  std::vector<BitmapInfoHeader> v;
  for (size_t i = 0; i != bitmaps.size(); i++){
    const auto& p = bitmaps[i];
    if (p.second == IcoCompression::PNG){
      auto pngStrIter = pngData.find(i);
      assert(pngStrIter != pngData.end());
      v.push_back(create_bitmap_info_header_png(p.first,
          pngStrIter->second.size()));
    }
    else {
      v.push_back(create_bitmap_info_header(p.first, 32, true));
    }
  }
  return v;
}

static std::vector<BitmapInfoHeader> create_bitmap_headers(const cur_vec& cursors){
  std::vector<BitmapInfoHeader> v;
  for (const auto& c : cursors){
    v.push_back(create_bitmap_info_header(c.first, 32, true));
  }
  return v;
}

static IconDir create_icon_dir_icon(const ico_vec& bitmaps){
  IconDir iconDir;
  iconDir.reserved = 0;
  iconDir.imageType = IconType::ICO;
  assert(bitmaps.size() <= UINT16_MAX);
  iconDir.imageCount = static_cast<uint16_t>(bitmaps.size());
  return iconDir;
}

static IconDir create_icon_dir_cursor(const cur_vec& cursors){
  IconDir iconDir;
  iconDir.reserved = 0;
  iconDir.imageType = IconType::CUR;
  assert(cursors.size() <= UINT16_MAX);
  iconDir.imageCount = static_cast<uint16_t>(cursors.size());
  return iconDir;
}

static std::vector<IconDirEntry> create_icon_dir_entries(const ico_vec& bitmaps,
  const std::map<size_t, std::string>& pngData)
{
  std::vector<IconDirEntry> v;

  // Fixme: Check casts
  int offset = static_cast<int>(struct_lengths<IconDir>()) +
    resigned(bitmaps.size()) * static_cast<int>(struct_lengths<IconDirEntry>());

  for (size_t i = 0; i != bitmaps.size(); i++){
    const auto& p = bitmaps[i];
    const auto& bmp = p.first;
    auto compression = p.second;

    IconDirEntry entry;
    set_size(entry, p.first.GetSize());
    entry.reserved = 0;
    entry.colorCount = 0; // 0 when >= 8bpp
    entry.colorPlanes = 1;
    entry.bpp = 32;

    if (compression == IcoCompression::PNG){
      const auto pngStrIter = pngData.find(i);
      assert(pngStrIter != pngData.end());
      
      entry.bytes = static_cast<uint32_t>(pngStrIter->second.size()); // Fixme: Check cast

      const size_t lengthOfHeaders = struct_lengths<
        BitmapInfoHeader, BitmapFileHeader>();

      // Offset to png-data, i.e. past both headers.
      entry.offset = static_cast<uint32_t>(offset + lengthOfHeaders); // Fixme: Check cast
      offset += static_cast<int>(entry.bytes + lengthOfHeaders); // Fixme: Check cast
    }
    else if (compression == IcoCompression::BMP){
      // Fixme: Check cast
      entry.bytes =
        static_cast<uint32_t>(area(bmp.GetSize()) * 4 +
        and_map_bytes(bmp.GetSize()) +
	struct_lengths<BitmapInfoHeader>()); // <- Fixme: Wrong?
      entry.offset = offset;
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
  std::vector<IconDirEntry> v;

  // Fixme: Check cast
  int offset = static_cast<int>(struct_lengths<IconDir>() +
    resigned(cursors.size()) * struct_lengths<IconDir>());

  for (size_t i = 0; i != cursors.size(); i++){
    const Bitmap& bmp = cursors[i].first;
    IconDirEntry entry;
    set_size(entry, bmp.GetSize());
    entry.reserved = 0;
    entry.colorCount = 0; // 0 when >= 8bpp
    set_hot_spot(entry, cursors[i].second);

    // Fixme: Check cast
    entry.bytes =
      static_cast<uint32_t>(area(bmp.GetSize()) * 4 +
      and_map_bytes(bmp.GetSize()) +
       struct_lengths<BitmapInfoHeader>());
    entry.offset = offset;
    v.push_back(entry);
    offset += entry.bytes;
  }
  return v;
}

BitmapFileHeader create_bitmap_file_header_png(size_t encodedSize){
  const int headerLengths = struct_lengths<BitmapInfoHeader, BitmapFileHeader>();

  BitmapFileHeader h;
  h.fileType = 0x4d42; // "BM" (reversed, endianness and all)
  h.fileLength = static_cast<uint32_t>(headerLengths + encodedSize); // Fixme: Check cast
  h.reserved1 = 0;
  h.reserved2 = 0;
  h.dataOffset = headerLengths;
  return h;
}

SaveResult write_ico(const FilePath& filePath,  const ico_vec& bitmaps){
  assert(!bitmaps.empty());
  if (bitmaps.size() > UINT16_MAX){
    return SaveResult::SaveFailed(error_ico_too_many_images(bitmaps.size()));
  }

  for (const auto& p : bitmaps){
    if (p.first.m_w > 256 || p.first.m_h > 256){
      return SaveResult::SaveFailed(utf8_string(
        "Maximum size for icons is 256x256"));
    }
  }

  std::map<size_t, std::string> pngData;
  for (size_t i = 0; i != bitmaps.size(); i++){
    const auto& p = bitmaps[i];
    if (p.second == IcoCompression::PNG){
      pngData[i] = to_png_string(p.first);
    }
  }

  const IconDir iconDir = create_icon_dir_icon(bitmaps);
  const auto iconDirEntries = create_icon_dir_entries(bitmaps,
    pngData);

  std::vector<BitmapInfoHeader> bmpHeaders = create_bitmap_headers(bitmaps,
    pngData);
  BinaryWriter out(filePath);
  if (!out.good()){
    return SaveResult::SaveFailed(error_open_file_write(filePath));
  }

  write_struct(out, iconDir);
  for (const auto& iconDirEntry : iconDirEntries){
    write_struct(out, iconDirEntry);
  }

  assert(bitmaps.size() == bmpHeaders.size());
  for (size_t i = 0; i != bitmaps.size(); i++){
    const auto& p = bitmaps[i];
    if (p.second == IcoCompression::PNG){
      const auto& pngStrIter = pngData.find(i);
      assert(pngStrIter != pngData.end());
      const auto& pngStr(pngStrIter->second);

      write_struct(out, create_bitmap_file_header_png(pngStr.size()));
      write_struct(out, bmpHeaders[i]);
      out.write(pngStr.c_str(), pngStr.size());
    }
    else{
      write_struct(out, bmpHeaders[i]);
      write_32bpp_BI_RGB_ICO(out, p.first);
    }
  }
  return SaveResult::SaveSuccessful();
}

SaveResult write_cur(const FilePath& filePath, const cur_vec& cursors){
  assert(!cursors.empty());
  if (cursors.size() > UINT16_MAX){
    return SaveResult::SaveFailed(error_ico_too_many_images(cursors.size()));
  }

  for (size_t i = 0; i != cursors.size(); i++){
    const Bitmap& bmp(cursors[i].first);
    if (bmp.m_w > 256 || bmp.m_h > 256){
      return SaveResult::SaveFailed(utf8_string(
        "Maximum size for cursors is 256x256"));
    }
  }
  IconDir iconDir = create_icon_dir_cursor(cursors);
  auto iconDirEntries(create_cursor_dir_entries(cursors));
  auto bmpHeaders(create_bitmap_headers(cursors));
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
    write_32bpp_BI_RGB_ICO(out, cursors[i].first);
  }
  return SaveResult::SaveSuccessful();
}

} // namespace
