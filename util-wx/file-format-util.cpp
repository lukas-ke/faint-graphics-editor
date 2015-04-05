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

#include <functional>
#include <algorithm>
#include <iterator>
#include "formats/file-formats.hh"
#include "text/formatting.hh"
#include "util-wx/file-format-util.hh"
#include "util/generator-adapter.hh"
#include "util/iter.hh"

namespace faint{

Formats built_in_file_formats(){
  return {
    format_gif(),
    format_ico(),
    format_cur(),
    format_load_bmp(),
    format_png(),
    format_save_bmp(BitmapQuality::COLOR_24BIT),
    format_save_bmp(BitmapQuality::COLOR_8BIT),
    format_save_bmp(BitmapQuality::GRAY_8BIT),
    format_wx_jpg()};
}

template<typename T>
T select(const T& src,
  const std::function<bool(const typename T::value_type&)>& func)
{
  T dst;
  std::copy_if(src.begin(), src.end(), back_inserter(dst), func);
  return dst;
}

Formats loading_file_formats(const Formats& allFormats){
  return select(allFormats, can_load_f);
}

Formats saving_file_formats(const Formats& allFormats){
  return select(allFormats, can_save_f);
}

static utf8_string wildcarded(const FileExtension& ext){
  return utf8_string("*.") + ext.Str();
}

static utf8_string extensions_filter(const std::vector<FileExtension>& exts){
  return join_fn(utf8_string(";"), exts, {wildcarded});
}

utf8_string file_dialog_filter(const Formats& formats){
  auto single_format_filter = [](const Format* f){
    return f->GetLabel() +
      utf8_string("|") +
      extensions_filter(f->GetExtensions());
  };

  return join_fn(utf8_string("|"), formats, {single_format_filter});
}

Optional<int> get_file_format_index(const Formats& formats,
  const FileExtension& extension)
{
  return find_index_if(formats,
    [&](auto f){return f->Match(extension);});
}

utf8_string combined_file_dialog_filter(const utf8_string& description,
  const Formats& formats)
{
  if (formats.empty()){
    return utf8_string("");
  }

  auto format_to_extensions_filter = [](const Format* const& f){
    return extensions_filter(f->GetExtensions());
  };

  return join(utf8_string("|"), description,
    join_fn(utf8_string(";"), formats, {format_to_extensions_filter}),
    file_dialog_filter(formats),
    utf8_string("All files"),
    utf8_string("*.*"));
}

static auto match_load(const FileExtension& ext){
  return [ext](Format* f){
    return f->Match(ext) && f->CanLoad();
  };
}

static auto match_save(const FileExtension& ext){
  return [ext](Format* f){
    return f->Match(ext) && f->CanSave();
  };
}

Optional<Format&> get_load_format(const Formats& formats,
  const FileExtension& ext)
{
  return find_if_deref(formats, match_load(ext));
}

Optional<Format&> get_save_format(const Formats& formats,
  const FileExtension& ext)
{
  return find_if_deref(formats, match_save(ext));
}

Optional<Format&> get_save_format(const Formats& formats,
  const FileExtension& ext, int filterIndex)
{
  if (0 < filterIndex && filterIndex < resigned(formats.size())){
    Format* f = formats[to_size_t(filterIndex)];
    if (f->Match(ext)){
      return Optional<Format&>(*f);;
    }
  }
  return get_save_format(formats, ext);
}

bool has_load_format(const Formats& formats, const FileExtension& ext){
  return any_of(formats, match_load(ext));
}

bool has_save_format(const Formats& formats, const FileExtension& ext){
  return any_of(formats, match_save(ext));
}

} // namespace
