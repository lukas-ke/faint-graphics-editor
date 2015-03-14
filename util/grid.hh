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

#ifndef FAINT_GRID_HH
#define FAINT_GRID_HH
#include "bitmap/color.hh"
#include "geo/point.hh"

namespace faint{

Color default_grid_color();
class Grid{
public:
  Grid(bool enabled=false, int spacing=10, const Color=default_grid_color());
  Point Anchor() const;
  Color GetColor() const;
  bool Enabled() const;
  bool Dashed() const;
  void SetAnchor(const Point&);
  void SetColor(const Color&);
  void SetDashed(bool);
  void SetEnabled(bool);
  void SetSpacing(int);
  Point Snap(const Point&) const;
  int Spacing() const;
private:
  Point m_anchor;
  Color m_color;
  bool m_dashed;
  bool m_enabled;
  int m_spacing;
};

} // namespace

#endif
