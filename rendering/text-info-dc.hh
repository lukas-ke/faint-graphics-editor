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

#ifndef FAINT_TEXT_INFO_DC_HH
#define FAINT_TEXT_INFO_DC_HH
#include "bitmap/bitmap.hh"
#include "rendering/faint-dc.hh"
#include "text/text-line.hh" // TextInfo
#include "util/optional.hh"

namespace faint{

class TextInfoDC : public TextInfo{
  // Uses a FaintDC to retrieve info about font properties.
public:
  explicit TextInfoDC(const Settings&);
  int GetWidth(const utf8_string&) const override;
  IntSize TextSize(const utf8_string& str) const override;
  int ComputeRowHeight() const override;

  coord Ascent() const;
  std::vector<int> CumulativeTextWidth(const utf8_string&);
  std::vector<PathPt> GetTextPath(const Tri& tri, const utf8_string&);
  FontMetrics GetFontMetrics();

  TextInfoDC& operator=(const TextInfoDC&) = delete;
  TextInfoDC(const TextInfoDC&) = delete;

private:
  Bitmap m_bitmap;
  FaintDC m_dc;
  const Settings& m_settings;
  mutable Optional<int> m_rowHeight;
};

} // namespace

#endif
