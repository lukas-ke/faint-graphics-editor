// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_SERIALIZE_BMP_PIXEL_DATA_HH
#define FAINT_SERIALIZE_BMP_PIXEL_DATA_HH

namespace faint{

class AlphaMap;
class BinaryReader;
class BinaryWriter;
class Bitmap;
class ColorList;

// Returns the required row-stride for the pixel-data rows with the
// specified bits per pixel and bitmap width.
int bmp_row_stride(int bpp, int w);

// Row stride for the and map in cursors, icons.
int and_map_row_stride(int w);

// The total bytes required for the and map for a cursor or icon of
// the given size.
int and_map_bytes(const IntSize& bmpSize);

template<int bpp>
int bmp_row_stride(int w){
  return bmp_row_stride(bpp, w);
}

template<int bpp>
int bmp_row_padding(int w){
  return bmp_row_stride(bpp, w) - (w * bpp) / 8;
}

void write_8bpp_BI_RGB(BinaryWriter&, const std::pair<AlphaMap, ColorList>&);
void write_24bpp_BI_RGB(BinaryWriter&, const Bitmap&);
void write_32bpp_BI_RGB_ICO(BinaryWriter&, const Bitmap&);

Optional<AlphaMap> read_1bpp_BI_RGB(BinaryReader&, const IntSize&);
Optional<AlphaMap> read_4bpp_BI_RGB(BinaryReader&, const IntSize&);
Optional<AlphaMap> read_8bpp_BI_RGB(BinaryReader&, const IntSize&);
Optional<Bitmap> read_24bpp_BI_RGB(BinaryReader&, const IntSize&);
Optional<Bitmap> read_32bpp_BI_RGB(BinaryReader&, const IntSize&);

Optional<ColorList> read_color_table(BinaryReader&, int numColors);

} // namespace

#endif
