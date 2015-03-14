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
#include "geo/geo-func.hh"
#include "geo/measure.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/pathpt.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objrectangle.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"
#include "util/setting-util.hh"

namespace faint{

class ObjRectangle : public Object{
public:
  ObjRectangle(const Tri& tri, const Settings& s)
    : Object(with_point_editing(s, start_enabled(false))),
      m_tri(tri)
  {}

  Object* Clone() const override{
    return new ObjRectangle(m_tri, m_settings);
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    dc.Rectangle(m_tri, m_settings);
  }

  void DrawMask(FaintDC& dc) override{
    dc.Rectangle(m_tri, mask_settings_fill(m_settings));
  }

  std::vector<Point> GetAttachPoints() const override{
    return get_attach_points(m_tri);
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return {
      PathPt::MoveTo(m_tri.P0()),
      PathPt::LineTo(m_tri.P1()),
      PathPt::LineTo(m_tri.P3()),
      PathPt::LineTo(m_tri.P2()),
      PathPt::PathCloser()
    };
  }

  std::vector<Point> GetMovablePoints() const override{
    return {GetHorizontalPoint()};
  }

  IntRect GetRefreshRect() const override{
    return floored(bounding_rect(m_tri, m_settings));
  }

  Tri GetTri() const override{
    return m_tri;
  }

  utf8_string GetType() const override{
    return "Rectangle";
  }

  Point GetPoint(int index) const override{
    assert(index == 0);
    return GetHorizontalPoint();
  }

  void SetPoint(const Point& p, int index) override{
    assert(index < 1);
    Point projected = projection(p, unbounded(P0_P1(m_tri)));

    LineSegment l(m_tri.P0(), m_tri.P2());
    if (side(projected, l) != side(m_tri.P1(), l)){
      m_settings.Set(ts_RadiusX, 0.0);
    }
    else{
      m_settings.Set(ts_RadiusX,
        std::min(distance(projected, m_tri.P0()), m_tri.Width() / 2.0));
    }
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  Point GetHorizontalPoint() const{
    auto radius = m_settings.Get(ts_RadiusX);
    return m_tri.P0() + Point(radius * cos(m_tri.GetAngle()),
      (radius * sin(m_tri.GetAngle())));
  }

  Tri m_tri;
};

Object* create_rectangle_object(const Tri& tri, const Settings& s){
  return new ObjRectangle(tri, s);
}

} // namespace
