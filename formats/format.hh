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

#ifndef FAINT_FORMAT_HH
#define FAINT_FORMAT_HH
#include "formats/save-result.hh"
#include "util/distinct.hh"
#include "util-wx/file-path.hh"

namespace faint{
class Canvas;
class ImageProps;

class category_format;
using label_t = Distinct<utf8_string, category_format, 0>;
using can_save = Distinct<bool, category_format, 0>;
using can_load = Distinct<bool, category_format, 1>;

class Format{
  // Base class for file formats for loading and saving
  // images in Faint.
public:
  Format(const FileExtension&,
    const label_t&, const can_save&, const can_load&);
  Format(const std::vector<FileExtension>&,
    const label_t&, const can_save&, const can_load&);
  virtual ~Format() = default;
  bool CanSave() const;
  bool CanLoad() const;
  const FileExtension& GetDefaultExtension() const;
  const std::vector<FileExtension>& GetExtensions() const;
  const utf8_string& GetLabel() const;
  bool Match(const FileExtension&) const;
  virtual void Load(const FilePath&, ImageProps&) = 0;
  virtual SaveResult Save(const FilePath&, Canvas&) = 0;
private:
  bool m_canLoad;
  bool m_canSave;
  std::vector<FileExtension> m_extensions;
  utf8_string m_label;
};

} // namespace

#endif
