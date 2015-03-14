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

#include "app/canvas.hh" // Fixme: Unfortunate (for GetObjects(), GetGrid())
#include "editors/line-editor.hh"
#include "geo/adjust.hh" // line_constrain
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "text/formatting.hh"
#include "util/grid.hh"
#include "util/object-util.hh" // snap
#include "util/pos-info.hh"

namespace faint{

static Point adjusted_p0(const PosInfo& info, const AllowSnap& allowSnap){
  return (info.modifiers.Primary() && allowSnap.Get()) ?
    snap(info.pos, info.canvas.GetObjects(), info.canvas.GetGrid()) :
    info.pos;
}

static Point adjusted_p1(const PosInfo& info,
  const Point& p0,
  const AllowConstrain& allowConstrain,
  const AllowSnap& allowSnap)
{
  if (info.modifiers.Primary() && allowSnap.Get()){
    return snap(info.pos, info.canvas.GetObjects(),
      info.canvas.GetGrid());
  }
  else if (info.modifiers.Secondary() && allowConstrain.Get()){
    return adjust_to_45(p0, info.pos);
  }
  else{
    return info.pos;
  }
}

LineEditor::LineEditor(AllowConstrain constrain, AllowSnap snap)
  : m_allowConstrain(constrain),
    m_allowSnap(snap)
{}

LineEditor::LineEditor(const PosInfo& info,
  AllowConstrain allowConstrain,
  AllowSnap allowSnap)
  : LineEditor(allowConstrain, allowSnap)
{
  MouseDown(info);
}

void LineEditor::MouseDown(const PosInfo& info){
  m_line = LineSegment(twice(adjusted_p0(info, m_allowSnap)));
}

void LineEditor::MouseMove(const PosInfo& info){
  m_line.p1 = adjusted_p1(info, m_line.p0, m_allowConstrain, m_allowSnap);
}

void LineEditor::MouseUp(const PosInfo& info){
  m_line.p1 = adjusted_p1(info, m_line.p0, m_allowConstrain, m_allowSnap);
}

LineSegment LineEditor::GetLine() const{
  return m_line;
}

IntRect LineEditor::GetRefreshRect() const{
  return floiled(bounding_rect(m_line));
}

void custom_line_status(StatusInterface& status,
  const LineSegment& line,
  const utf8_string& description)
{
  status.SetMainText(description);
  status.SetText(str_line_status(floored(line)), 0);
}

void default_line_status(const PosInfo& info, const LineEditor& e){
  custom_line_status(info.status,
    e.GetLine(),
    info.modifiers.Neither() ?
      space_sep(primary_modifier("Snap"), secondary_modifier("Constrain")) :
      "");
}

} // namespace
