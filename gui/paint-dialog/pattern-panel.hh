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

#ifndef FAINT_PATTERN_PANEL_HH
#define FAINT_PATTERN_PANEL_HH
#include "bitmap/pattern.hh"
#include "gui/common-cursors.hh"
#include "util/accessor.hh"

class wxWindow;

namespace faint{

class CommonCursors;

class PaintPanel_Pattern{
public:
  PaintPanel_Pattern(wxWindow* parent,
    const CommonCursors&,
    const Getter<Bitmap>& getBitmap);
  ~PaintPanel_Pattern();
  wxWindow* AsWindow();
  void Copy();
  Pattern GetPattern() const;
  void Paste();
  void SetPattern(const Pattern&);

  PaintPanel_Pattern(const PaintPanel_Pattern&) = delete;
  PaintPanel_Pattern& operator=(const PaintPanel_Pattern&) = delete;
private:
  class PaintPanel_Pattern_Impl;
  PaintPanel_Pattern_Impl* m_impl;
};

} // namespace

#endif
