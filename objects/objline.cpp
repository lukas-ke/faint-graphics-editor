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
#include <cassert>
#include "geo/arrowhead.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objline.hh"
#include "rendering/faint-dc.hh"
#include "text/formatting.hh"
#include "util/iter.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"
#include "util/setting-util.hh"
#include "util/settings.hh"

namespace faint{

class ObjLine : public Object{
public:
  ObjLine(const Points& points, const Settings& settings)
    : Object(with_point_editing(settings, start_enabled(points.Size() == 2))),
      m_lastIndex(0),
      m_points(points),
      m_tri(points.GetTri())
  {
    assert(m_points.Size() >= 2);
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    dc.PolyLine(m_tri, m_points.GetPointsDumb(m_tri), m_settings);
  }

  void DrawMask(FaintDC& dc) override{
    dc.PolyLine(m_tri, m_points.GetPointsDumb(m_tri),
      mask_settings_line(m_settings));
  }

  IntRect GetRefreshRect() const override{
    if (m_settings.Get(ts_LineArrowhead) == LineArrowhead::FRONT){
      return floored(union_of(GetLineRect(), GetArrowHeadRect()));
    }
    return floored(GetLineRect());
  }

  utf8_string GetType() const override{
    return "Line";
  }

  bool HitTest(const Point& pt) override{
    if (m_settings.Get(ts_LineArrowhead) == LineArrowhead::FRONT){
      return GetLineRect().Contains(pt) || GetArrowHeadRect().Contains(pt);
    }
    return GetLineRect().Contains(pt);
  }

  std::vector<Point> GetMovablePoints() const override{
    return m_points.GetPointsDumb(m_tri);
  }

  Tri GetTri() const override{
    return m_tri;
  }

  std::vector<Point> GetAttachPoints() const override{
    std::vector<Point> pts(m_points.GetPointsDumb(m_tri));
    std::vector<Point> extension(mid_points(pts));
    pts.insert(end(pts), begin(extension), end(extension));
    return pts;
  }

  std::vector<Point> GetExtensionPoints() const override{
    return mid_points(m_points.GetPointsDumb(m_tri));
  }

  Object* Clone() const override{
    return new ObjLine(*this);
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    const std::vector<Point> pts(m_points.GetPointsDumb(m_tri));
    std::vector<PathPt> path;
    path.push_back(PathPt::MoveTo(pts[0]));
    for (auto pt : but_first(pts)){
      path.push_back(PathPt::LineTo(pt));
    }
    return path;
  }

  Point GetPoint(int index) const override{
    assert(index < m_points.Size());
    return m_points.GetPointsDumb(m_tri)[to_size_t(index)];
  }

  int NumPoints() const override{
    return m_points.Size();
  }

  void SetPoint(const Point& pt, int index) override{
    m_lastIndex = index;
    m_points.SetPoint(m_tri, pt, index);
    SetTri(m_points.GetTri()); // Fixme
  }

  void InsertPoint(const Point& pt, int index) override{
    m_points.InsertPoint(GetTri(), pt, index);
    SetTri(m_points.GetTri());
  }

  utf8_string StatusString() const override{
    return str_line_status_subpixel(
      LineSegment((m_lastIndex == 0 ?
          GetPoint(m_points.Size() - 1) :
          GetPoint(m_lastIndex - 1)),
        GetPoint(m_lastIndex)));
  }

  bool CanRemovePoint() const override{
    return m_points.Size() > 2;
  }

  bool Extendable() const override{
    return true;
  }

  void RemovePoint(int index) override{
    m_points.RemovePoint(m_tri, index);
    SetTri(m_points.GetTri());
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  ObjLine(const ObjLine& other)
    : Object(other.m_settings),
      m_lastIndex(0),
      m_points(other.m_points),
      m_tri(other.GetTri())
  {}

  Rect GetLineRect() const {
    coord lineWidth(std::max(m_settings.Get(ts_LineWidth), 6.0));
    return inflated(bounding_rect(GetTri()), lineWidth / 2.0);
  }

  Rect GetArrowHeadRect() const {
    return get_arrowhead(LineSegment(m_tri.P0(), m_tri.P1()),
      m_settings.Get(ts_LineWidth)).BoundingBox();
  }

  int m_lastIndex;
  Points m_points;
  Tri m_tri;
};

Object* create_line_object(const Points& points, const Settings& s){
  return new ObjLine(points, s);
}

bool is_line(const Object* obj){
  return dynamic_cast<const ObjLine*>(obj) != nullptr;
}

} // namespace
