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

#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "util/grid.hh"

namespace faint{

Color default_grid_color(){
  return Color(100,100,255,150);
}

Grid::Grid(bool enabled, int spacing, const Color color)
  : m_color(color),
    m_dashed(true),
    m_enabled(enabled),
    m_spacing(spacing)
{}

Point Grid::Anchor() const{
  return m_anchor;
}

Color Grid::GetColor() const{
  return m_color;
}

bool Grid::Dashed() const{
  return m_dashed;
}

bool Grid::Enabled() const{
  return m_enabled;
}

void Grid::SetAnchor(const Point& point){
  m_anchor = point;
}

void Grid::SetColor(const Color& color){
  m_color = color;
}

void Grid::SetDashed(bool dashed){
  m_dashed = dashed;
}

void Grid::SetEnabled(bool enabled){
  m_enabled = enabled;
}

void Grid::SetSpacing(int spacing){
  m_spacing = spacing;
}

static Point round_up(const Point& p){
  return floated(IntPoint(floored(p.x + 0.5),
    floored(p.y + 0.5)));
}

Point Grid::Snap(const Point& p) const{
  Point translate = m_anchor % floated(m_spacing);
  Point relativeGrid = (p - translate) / floated(m_spacing);
  return round_up(relativeGrid) * m_spacing + translate;
}

int Grid::Spacing() const{
  return m_spacing;
}

} // namespace
