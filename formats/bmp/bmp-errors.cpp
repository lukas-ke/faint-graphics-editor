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

#include "formats/bmp/bmp-errors.hh"
#include "formats/bmp/bmp-types.hh"
#include "text/formatting.hh"
#include "util-wx/file-path.hh"

namespace faint{

template<typename T>
typename std::underlying_type<T>::type enum_value(T e){
  return static_cast<typename std::underlying_type<T>::type>(e);
}

utf8_string error_ico_too_many_images(size_t frames){
  return endline_sep(
    "Too many frames for saving as icon.\n",
    space_sep(str_uint(frames), " > ", str_uint(static_cast<size_t>(UINT16_MAX))));
}

utf8_string error_bitmap_signature(uint16_t v){
  return endline_sep(
    "Faint can not read this file.\n",
    space_sep(lbl_u("Bitmap signature:", v),
      "expected", str_int(v)));
}

utf8_string error_bpp(size_t num, int bpp){
  return endline_sep(
    "The bits per pixel setting for this icon is not supported by Faint.\n",
    lbl_u("Image entry", num + 1),
    lbl("Bits per pixel", bpp));
}

utf8_string error_color_planes(size_t num, int planes){
  return endline_sep(
    "The number of color planes for this icon is invalid.\n",
    lbl_u("Image entry", num + 1),
    space_sep(lbl("Color planes", planes),
      "(expected 1)"));
}

utf8_string error_compression(size_t num, Compression compression){
  return endline_sep(
    "The compression for this icon is not supported by Faint.\n",
    lbl_u("Icon#", num + 1),
    space_sep(lbl("Compression", enum_value(compression)),
      enum_str(compression)));
}

utf8_string error_dir_reserved(int value){
  return endline_sep("This icon appears broken.\n",
    space_sep("The reserved entry of the IconDir is",
      str_int(value),
      "(expected: 0)"));
}

utf8_string error_image(size_t num){
  return endline_sep("This icon appears broken.\n",
    space_sep("Failed reading the header for image entry",
      str_uint(num + 1)));
}

utf8_string error_icon_is_cursor(){
  // Fixme: Consider allowing. Maybe make it a warning
  return "This supposed icon file contains cursors.";
}

utf8_string error_cursor_is_icon(){
  // Fixme: Consider allowing. Maybe make it a warning
  return "Error: This supposed cursor file file contains.";
}

utf8_string error_open_file_read(const FilePath& path){
  return endline_sep("The file could not be opened for reading.",
    lbl("Filename", path.Str()));
}

utf8_string error_open_file_write(const FilePath& path){
  return endline_sep("The file could not be opened for writing.",
    lbl("Filename", path.Str()));
}

utf8_string error_premature_eof(const char* structureName){
  return endline_sep("This icon appears broken.",
    space_sep("Reading the", structureName, "failed."));
}

utf8_string error_no_images(){
  return "This icon contains no images.\n\n"
    "The IconDir images entry is 0.";
}

utf8_string error_truncated_bmp_header(size_t num, int len){
  return endline_sep("This icon file appears broken.\n",
    lbl_u("Image entry", num + 1),
    space_sep(lbl("Header length", len),
      bracketed(space_sep("expected", str_int(40))))); // FIXME
}

utf8_string error_bmp_data(size_t num){
  return endline_sep("This icon file appears broken.\n",
    "Reading the data for a bitmap failed.",
    lbl_u("Image entry", num + 1));
}

utf8_string error_truncated_png_data(const Index& num){
  return endline_sep("This icon file appears broken.\n",
    "Reading an embedded png-compressed image failed.",
    lbl_u("Image entry", to_size_t(num) + 1));
}

utf8_string error_read_to_offset(size_t num, int offset){
  return endline_sep("This icon file appears broken.\n",
    "Seeking to an image offset failed.",
    lbl_u("Image entry", num),
    lbl("At", str_hex(offset)));
}

utf8_string error_unknown_image_type(IconType type){
  return endline_sep(
   "The image type in this icon file is not recognized by Faint.\n",
   space_sep(lbl("Image type", enum_value(type)),
     "(expected 1 for icon or 2 for cursor)."));
}

} // namespace
