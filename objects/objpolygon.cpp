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

#include <cassert>
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/measure.hh" // with_mid_points_cyclic
#include "geo/pixel-snap.hh"
#include "geo/points.hh"
#include "objects/standard-object.hh"
#include "objects/objpolygon.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/object-util.hh"
#include "util/setting-util.hh"
#include "util/type-util.hh"

namespace faint{

class ObjPolygon : public StandardObject{
public:
  ObjPolygon(const Points& points, const Settings& settings)
    : StandardObject(with_point_editing(settings, start_enabled(false))),
      m_points(points),
      m_tri(points.GetTri())
  {
    assert(!m_points.Empty());
    assert(m_points.Front().IsMove());
  }

  Object* Clone() const override{
    return new ObjPolygon(*this);
  }

  bool CyclicPoints() const override{
    return true;
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    dc.Polygon(m_tri, m_points.GetPointsDumb(m_tri), m_settings);
  }

  void DrawMask(FaintDC& dc, ExpressionContext&) override{
    dc.Polygon(m_tri, m_points.GetPointsDumb(m_tri),
      mask_settings_fill(m_settings));
  }

  coord GetArea() const override{
    return area(m_tri); // Fixme: Incorrect, need proper polygon area
  }

  std::vector<Point> GetAttachPoints() const override{
    return with_mid_points_cyclic(Vertices());
  }

  std::vector<ExtensionPoint> GetExtensionPoints() const override{
    std::vector<Point> points = Vertices();
    if (points.empty()){
      return {};
    }

    // Need to include the first point at the end, to get the midpoint on
    // the closing segment
    points.push_back(points[0]);
    return extension_index_from(1, mid_points(points));
  }

  std::vector<Point> GetMovablePoints() const override{
    return Vertices();
  }

  IntRect GetRefreshRect() const override{
    return floored(bounding_rect_ink(m_tri, m_settings));
  }

  Tri GetTri() const override{
    return m_tri;
  }

  utf8_string GetType() const override{
    return "Polygon";
  }

  Point GetPoint(int index) const override{
    assert(index < m_points.Size());
    return Vertices()[to_size_t(index)];
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    std::vector<PathPt> points(m_points.GetPoints(m_tri));
    points.push_back(PathPt::PathCloser());
    return points;
  }

  void SetPoint(const Point& pt, int index) override{
    assert(index >= 0);
    m_points.SetPoint(m_tri, pt, index);
    SetTri(m_points.GetTri()); // Fixme
  }

  UndoAddFunc InsertPoint(const Point& pt, int index) override{
    assert(index >= 0);
    Tri oldTri = m_tri;
    auto undoFunc = [=](){
      m_points.RemovePoint(oldTri, index);
      SetTri(oldTri);
    };

    m_points.InsertPoint(m_tri, pt, index);
    SetTri(m_points.GetTri());
    return undoFunc;
  }

  void RemovePoint(int index) override{
    assert(index >= 0);
    m_points.RemovePoint(m_tri, index);
    SetTri(m_points.GetTri());
  }

  int NumPoints() const override{
    return m_points.Size();
  }

  Optional<CmdFuncs> PixelSnapFunc(){
    return {CmdFuncs(
      [this](){
        m_points = Points(pixel_snap(m_points.GetPoints(m_tri),
          m_settings.Get(ts_LineWidth)));
        m_tri = m_points.GetTri();
      },
      [oldPoints=m_points, oldTri=m_tri, this](){
        m_points = oldPoints;
        m_tri = oldTri;

      })};
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

  const std::vector<Point> Vertices() const {
    return m_points.GetPointsDumb(GetTri());
  }

  bool Extendable() const override{
    return true;
  }

  bool CanRemovePoint() const override{
    return m_points.Size() > 3;
  }

private:
  ObjPolygon(const ObjPolygon& other)
    : StandardObject(other.m_settings),
      m_points(other.m_points),
      m_tri(other.GetTri())
  {}

  Points m_points;
  Tri m_tri;
};

Object* create_polygon_object_raw(const Points& points, const Settings& s){
  return new ObjPolygon(points,s);
}

ObjectPtr create_polygon_object(const Points& points, const Settings& s){
  return std::make_unique<ObjPolygon>(points,s);
}

bool is_polygon(const Object* obj){
  return is_polygon(*obj);
}

bool is_polygon(const Object& obj){
  return is_type<ObjPolygon>(obj);
}

std::vector<Point> get_polygon_vertices(const Object* obj){
  auto polygon = dynamic_cast<const ObjPolygon*>(obj);
  assert(polygon != nullptr);
  return polygon->Vertices();
}

std::vector<Point> get_polygon_vertices(const Object& obj){
  return get_polygon_vertices(&obj);
}

} // namespace
