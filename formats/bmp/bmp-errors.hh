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

#ifndef FAINT_BMP_ERRORS_HH
#define FAINT_BMP_ERRORS_HH
#include "util/index.hh"
#include "text/utf8-string.hh"
#include "formats/bmp/bmp-types.hh"

namespace faint{ class FilePath; }

namespace faint{

utf8_string error_ico_too_many_images(size_t);
utf8_string error_bitmap_signature(uint16_t);
utf8_string error_bpp(size_t num, int bpp);
utf8_string error_color_planes(size_t num, int planes);
utf8_string error_compression(size_t num, Compression);
utf8_string error_dir_reserved(int value);
utf8_string error_image(size_t num);
utf8_string error_icon_is_cursor();
utf8_string error_cursor_is_icon();
utf8_string error_open_file_read(const FilePath&);
utf8_string error_open_file_write(const FilePath& path);
utf8_string error_premature_eof(const char* structureName);
utf8_string error_no_images();
utf8_string error_truncated_bmp_header(size_t num, int len);
utf8_string error_bmp_data(size_t num);
utf8_string error_truncated_png_data(const Index&);
utf8_string error_read_to_offset(size_t num, int offset);
utf8_string error_unknown_image_type(IconType);

} // namespace

#endif
