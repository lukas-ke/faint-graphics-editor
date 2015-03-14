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

#ifndef FAINT_LINE_EDITOR_HH
#define FAINT_LINE_EDITOR_HH
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "util/distinct.hh"

namespace faint{

class utf8_string;
class StatusInterface;
class PosInfo;

class category_line_editor;
using AllowConstrain = Distinct<bool, category_line_editor, 0>;
using AllowSnap = Distinct<bool, category_line_editor, 1>;

class LineEditor{
  // A helper for tools that draw lines.
public:
  LineEditor(AllowConstrain, AllowSnap);
  LineEditor(const PosInfo&, AllowConstrain, AllowSnap);
  LineSegment GetLine() const;
  IntRect GetRefreshRect() const;

  void MouseDown(const PosInfo&);
  void MouseMove(const PosInfo&);
  void MouseUp(const PosInfo&);
private:
  AllowConstrain m_allowConstrain;
  AllowSnap m_allowSnap;
  LineSegment m_line;
};

void custom_line_status(StatusInterface&,
  const LineSegment&,
  const utf8_string& description);

void default_line_status(const PosInfo&, const LineEditor&);


} // namespace

#endif
