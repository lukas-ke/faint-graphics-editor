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

#include "geo/pathpt.hh"
#include "rendering/text-info-dc.hh"

namespace faint{

TextInfoDC::TextInfoDC(const Settings& settings)
  : m_bitmap(IntSize(10,10)),
    m_dc(m_bitmap),
    m_settings(settings)
{}

int TextInfoDC::GetWidth(const utf8_string& str) const{
  return m_dc.TextSize(str, m_settings).w;
}

IntSize TextInfoDC::TextSize(const utf8_string& str) const{
  return m_dc.TextSize(str, m_settings);
}

int TextInfoDC::ComputeRowHeight() const{
  // A full row is the ascent + descent
  if (m_rowHeight.NotSet()){
    auto metrics = m_dc.GetFontMetrics(m_settings);
    m_rowHeight.Set(metrics.ascent + metrics.descent);
  }
  return m_rowHeight.Get();
}

coord TextInfoDC::Ascent() const{
  return floated(m_dc.GetFontMetrics(m_settings).ascent);
}

std::vector<int> TextInfoDC::CumulativeTextWidth(const utf8_string& str){
  return m_dc.CumulativeTextWidth(str, m_settings);
}

std::vector<PathPt> TextInfoDC::GetTextPath(const Tri& tri,
  const utf8_string& str)
{
  return m_dc.GetTextPath(tri, str, m_settings);
}

FontMetrics TextInfoDC::GetFontMetrics(){
  return m_dc.GetFontMetrics(m_settings);
}

} // namespace
