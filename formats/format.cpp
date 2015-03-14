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

#include <algorithm>
#include "formats/format.hh"

namespace faint{

Format::Format(const FileExtension& ext,
  const label_t& label,
  const can_save& canSave,
  const can_load& canLoad)
  : m_canLoad(canLoad.Get()),
    m_canSave(canSave.Get()),
    m_extensions({ext}),
    m_label(label.Get())
{}

Format::Format(const std::vector<FileExtension>& extensions,
  const label_t& label,
  const can_save& canSave,
  const can_load& canLoad)
  : m_canLoad(canLoad.Get()),
    m_canSave(canSave.Get()),
    m_extensions(extensions),
    m_label(label.Get())
{}

bool Format::CanLoad() const{
  return m_canLoad;
}

bool Format::CanSave() const{
  return m_canSave;
}

const FileExtension& Format::GetDefaultExtension() const{
  return m_extensions.front();
}

const std::vector<FileExtension>& Format::GetExtensions() const{
  return m_extensions;
}

const utf8_string& Format::GetLabel() const{
  return m_label;
}

bool Format::Match(const FileExtension& e) const {
  return std::find(begin(m_extensions), end(m_extensions), e)
    != m_extensions.end();
}

} // namespace
