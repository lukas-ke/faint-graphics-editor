// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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
#include "formats/bmp/bmp-types.hh"
#include "formats/bmp/serialize-bmp-types.hh"
#include "geo/limits.hh"

namespace faint{

BmpSizeAndOrder get_size_and_order(const BitmapInfoHeader& h){
  return h.height < 0 ?
    BmpSizeAndOrder{IntSize(h.width, -h.height), true} :
    BmpSizeAndOrder{IntSize(h.width, h.height), false};
}

const char* enum_str(Compression c){
  switch (c){
  case Compression::BI_RGB: return "BI_RGB";
  case Compression::BI_RLE8: return "BI_RLE8";
  case Compression::BI_RLE4: return "BI_RLE4";
  case Compression::BI_BITFIELDS: return "BI_BITFIELDS";
  case Compression::BI_JPEG: return "BI_JPEG";
  case Compression::BI_PNG: return "BI_PNG";
  case Compression::BI_ALPHABITFIELDS: return "BI_ALPHABITFIELDS";
  default: return "Unknown";
  }
}

const char* enum_str(IconType t){
  switch (t){
  case IconType::ICO: return "ICO";
  case IconType::CUR: return "CUR";
  default: return "Unknown";
  }
}

static int from_icon_measure(uint8_t measure){
  return measure == 0 ? 256 : static_cast<int>(measure);
}

IntSize get_size(const IconDirEntry& e){
  return IntSize(from_icon_measure(e.width), from_icon_measure(e.height));
}

static uint8_t to_icon_measure(int value){
  assert(value <= 256);
  return value == 256 ? 0 : static_cast<uint8_t>(value);
}

void set_size(IconDirEntry& e, const IntSize& sz){
  e.width = to_icon_measure(sz.w);
  e.height = to_icon_measure(sz.h);
}

HotSpot get_hot_spot(const IconDirEntry& e){
  // Color-planes and bits per pixel are appropriated for
  // the hot spot-location in the cursor format.
  return HotSpot(e.colorPlanes, e.bitsPerPixel);
}

bool set_hot_spot(IconDirEntry& e, const HotSpot& p){
  // See get_hot_spot-note.
  const bool validHotspot =
    can_represent<decltype(IconDirEntry::colorPlanes)>(p.x) &&
    can_represent<decltype(IconDirEntry::bitsPerPixel)>(p.y);
  if (validHotspot){
    e.colorPlanes = convert(p.x);
    e.bitsPerPixel = convert(p.y);
    return true;
  }
  else{
    e.colorPlanes = 0;
    e.bitsPerPixel = 0;
    return false;
  }
}

static int32_t to_pixels_per_meter(const DPI& dpi){
  const coord inchesPerMeter = 39.3701;
  return static_cast<int32_t>(dpi.Get() * inchesPerMeter + 0.5);
}

DPI default_DPI(){
  return DPI(96); // Rather arbitrarily chosen.
}

static BitmapInfoHeader create_bitmap_info_header(const IntSize& size,
  uint16_t bitsPerPixel,
  const DPI& dpi,
  const PaletteColors& paletteColors,
  bool andMap)
{
  BitmapInfoHeader h;
  h.headerLen = struct_lengths<BitmapInfoHeader>();
  h.width = size.w;
  h.height = andMap ? 2*size.h : size.h;
  h.colorPlanes = 1;
  h.bitsPerPixel = bitsPerPixel;
  h.compression = Compression::BI_RGB;
  h.rawDataSize = 0; // Dummy value 0 allowed for BI_RGB
  h.horizontalResolution = h.verticalResolution = to_pixels_per_meter(dpi);
  h.paletteColors = paletteColors.Get();
  h.importantColors = 0;
  return h;
}

BitmapInfoHeader create_bitmap_info_header_8bipp(const IntSize& size,
  const DPI& dpi,
  const PaletteColors& paletteColors,
  bool andMap)
{
  return create_bitmap_info_header(size,
    8,
    dpi,
    paletteColors,
    andMap);
}

BitmapInfoHeader create_bitmap_info_header_24bipp(const IntSize& size,
  const DPI& dpi,
  bool andMap)
{
  return create_bitmap_info_header(size,
    24,
    dpi,
    PaletteColors(0),
    andMap);
}

BitmapInfoHeader create_bitmap_info_header_32bipp(const IntSize& size,
  const DPI& dpi,
  bool andMap)
{
  return create_bitmap_info_header(size,
    32,
    dpi,
    PaletteColors(0),
    andMap);
}

BitmapInfoHeader create_bitmap_info_header_png(const IntSize& bmpSize,
  size_t rawDataSize,
  const DPI& dpi)
{
  BitmapInfoHeader h;
  h.headerLen = struct_lengths<BitmapInfoHeader>();
  h.width = bmpSize.w;
  h.height = bmpSize.h;
  h.colorPlanes = 1;
  h.bitsPerPixel = 0;
  h.compression = Compression::BI_PNG;
  h.rawDataSize = convert(rawDataSize);
  h.horizontalResolution = h.verticalResolution = to_pixels_per_meter(dpi);
  h.paletteColors = 0;
  h.importantColors = 0;
  return h;
}

} // namespace
