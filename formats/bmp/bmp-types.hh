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

#ifndef FAINT_BMP_TYPES_HH
#define FAINT_BMP_TYPES_HH
#include <cstdint>
#include <string>
#include <array>
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "util/deserialize-tuple.hh"
#include "util/hot-spot.hh"
#include "util/serialize-tuple.hh"
#include "util/distinct.hh"

namespace faint {

const uint16_t BITMAP_SIGNATURE = 0x4d42; // "BM", little endian

class BitmapFileHeader{
  // Information about the type, size, and layout of a file that
  // contains a "DIB".
public:
  uint16_t fileType; // Must be "BM"
  uint32_t fileLength; // Length of file, in bytes.
  uint16_t reserved1; // Must be 0
  uint16_t reserved2; // Must be 0

  // Offset in bytes from the beginning of BitmapFileHeader to the
  // pixel data.
  uint32_t dataOffset;
};

enum class Compression : uint32_t {
  BI_RGB = 0,
  BI_RLE8 = 1,
  BI_RLE4 = 2,
  BI_BITFIELDS = 3,
  BI_JPEG = 4,
  BI_PNG = 5,
  BI_ALPHABITFIELDS = 6,
};

// Returns the compression name (e.g. "BI_RGB") - or "Unknown" if
// invalid.
const char* enum_str(Compression);

class BitmapInfoHeader{
  // A Windows BITMAPINFOHEADER (a.k.a. DIB-header).
public:
  uint32_t headerLen;
  int32_t width;
  int32_t height; // Note: Negative height means top-down
  uint16_t colorPlanes;
  uint16_t bitsPerPixel;
  Compression compression;
  uint32_t rawDataSize;
  int32_t horizontalResolution;
  int32_t verticalResolution;
  uint32_t paletteColors;
  uint32_t importantColors;
};

class BmpSizeAndOrder{
public:
  // Note: The height of this size shall be positive regardless of
  // top-down/bottom-up pixel row order.
  IntSize size;

  // Pixel row order in bitmap images,
  //
  // True if top-down - the TOP-left pixel is the first value of
  // the row with the lowest address.
  //
  // False if bottom-up - the BOTTOM-left pixel is the first value
  // of the row with the lowest address.
  bool topDown;
};

// Extract the size of the bitmap and the row order from a BitmapInfoHeader.
//
// Note: The size will have positive height regardless of row order
// (unlike in the BitmapInfoHeader where negative height indicates a
// top-down bitmap).
BmpSizeAndOrder get_size_and_order(const BitmapInfoHeader&);

enum class IconType : uint16_t {
  ICO = 1,
  CUR = 2
};

const char* enum_str(IconType);

class IconDir{
  // The top-most header in an icon file.
public:
  uint16_t reserved; // Reserved, must always be 0
  IconType imageType;
  uint16_t imageCount; // Number of images in file
};

class IconDirEntry{
  // A list of these follow the IconDir-header for ico and cur.
public:
  uint8_t width; // Note: 0 should be interpreted as 256,
  uint8_t height; // Note: 0 should be interpreted as 256,
  uint8_t colorCount; // Number of colors in image (0 if bitsPerPixel >= 8)
  uint8_t reserved;
  uint16_t colorPlanes; // 0 or 1
  uint16_t bitsPerPixel;

  // The size of the image's data in bytes (Fixme: Including headers?)
  uint32_t bytes;

  // The offset to the bmp-header or png-data from the beginning of
  // the Ico/Cur file.
  uint32_t offset;
};

IntSize get_size(const IconDirEntry&);

// Sets the size for the entry.
// Note: Asserts if the width or height is > 256
void set_size(IconDirEntry&, const IntSize&);

// Cursors only: Gets the hot-spot for this entry.
HotSpot get_hot_spot(const IconDirEntry&);

// Cursors only: Sets the hot-spot for this entry
// Returns true if successful, false if the HotSpot was out of range.
// - When the hot-spot is out of range, it is set to 0, 0
bool set_hot_spot(IconDirEntry&, const HotSpot&);

class category_bmp_types;

// Dots-per-inch data type
using DPI = Distinct<int, category_bmp_types, 0>;
using PaletteColors = Distinct<uint32_t, category_bmp_types, 1>;

// Returns a  rather arbitrarily chosen dots-per-inch value.
DPI default_DPI();

BitmapInfoHeader create_bitmap_info_header_8bipp(const IntSize&,
  const DPI&,
  const PaletteColors&,
  bool andMap
);

BitmapInfoHeader create_bitmap_info_header_24bipp(const IntSize&,
  const DPI&,
  bool andMap
);

BitmapInfoHeader create_bitmap_info_header_32bipp(const IntSize&,
  const DPI&,
  bool andMap
);

BitmapInfoHeader create_bitmap_info_header_png(const IntSize&,
  size_t rawDataSize,
  const DPI&);

} // namespace

#endif
