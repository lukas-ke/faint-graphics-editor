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
#include "geo/int-rect.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objspline.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/setting-util.hh"

namespace faint{

class ObjSpline : public Object{
public:
  ObjSpline(const Points& pts, const Settings& settings)
    : Object(settings),
      m_points(pts),
      m_tri(pts.GetTri())
  {}

  Object* Clone() const override{
    return new ObjSpline(*this);
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    dc.Spline(m_points.GetPointsDumb(m_tri), m_settings);
  }

  void DrawMask(FaintDC& dc) override{
    dc.Spline(m_points.GetPointsDumb(m_tri), mask_settings_line(m_settings)) ;
  }

  std::vector<Point> GetAttachPoints() const override{
    std::vector<Point> splinePts(m_points.GetPointsDumb(m_tri));
    return{ splinePts.front(), splinePts.back() };
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return spline_to_svg_path(m_points.GetPointsDumb(m_tri));
  }

  IntRect GetRefreshRect() const override{
    return floored(inflated(bounding_rect(m_tri), m_settings.Get(ts_LineWidth)));
  }

  utf8_string GetType() const override{
    return "Spline";
  }

  std::vector<Point> GetSplinePoints() const {
    return m_points.GetPointsDumb(m_tri);
  }

  Tri GetTri() const override{
    return m_tri;
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

  bool ShowSizeBox() const override{
    return true;
  }

private:
  ObjSpline(const ObjSpline& other)
    : Object(other.m_settings),
      m_points(other.m_points),
      m_tri(other.GetTri())
  {}

  Points m_points;
  Tri m_tri;
};

Object* create_spline_object(const Points& points, const Settings& s){
  return new ObjSpline(points, s);
}

bool is_spline(Object* obj){
  return dynamic_cast<ObjSpline*>(obj) != nullptr;
}

std::vector<Point> get_spline_points(Object* obj){
  ObjSpline* spline = dynamic_cast<ObjSpline*>(obj);
  assert(spline != nullptr);
  return spline->GetSplinePoints();
}

std::vector<PathPt> spline_to_svg_path(const std::vector<Point>& points){
  std::vector<PathPt> out;
  const Point& p0 = points[0];
  coord x1 = p0.x;
  coord y1 = p0.y;

  const Point& p1 = points[1];
  coord c = p1.x;
  coord d = p1.y;

  coord x3 = (x1 + c) / 2;
  coord y3 = (y1 + d) / 2;

  out.push_back(PathPt::MoveTo(Point(x1, y1)));
  out.push_back(PathPt::LineTo(Point(x3, y3)));

  for (size_t i = 2; i != points.size(); i++){
    x1 = x3;
    y1 = y3;
    coord x2 = c;
    coord y2 = d;

    const Point& pt = points[i];
    c = pt.x;
    d = pt.y;
    x3 = (x2 + c) / 2;
    y3 = (y2 + d) / 2;
    out.push_back(PathPt::CubicBezierTo(Point(x3, y3),
        Point(x1, y1),
        Point(x2, y2)));
  }
  return out;
}

} // namespace
