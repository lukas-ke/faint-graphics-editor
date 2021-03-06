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

#include "geo/arc.hh"
#include "geo/geo-func.hh"
#include "geo/geo-list-points.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"
#include "geo/tri.hh"
#include "objects/standard-object.hh"
#include "objects/objellipse.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/object-util.hh"
#include "util/optional.hh"
#include "util/setting-id.hh"
#include "util/setting-util.hh"
#include "util/type-util.hh"

namespace faint{

static void draw_ellipse_span(FaintDC& dc,
  const Tri& tri,
  const AngleSpan& angles,
  const Settings& settings)
{
  if (angles.Empty()){
    dc.Ellipse(tri, settings);
  }
  else{
    dc.Arc(tri, angles, settings);
  }
}

static Angle stretch_angle(const Angle& a, const Radii& r){
  if (r.y == 0){
    return 0_deg;
  }
  // Turn the circular angle into an elliptic angle
  return atan2((r.x * r.x / r.y) * sin(a), r.x * cos(a));
}

class ObjEllipse : public StandardObject{
public:
  ObjEllipse(const Tri& tri, const Settings& settings)
    : StandardObject(with_point_editing(settings, start_enabled(false))),
      m_angleSpan(Angle::Zero(), Angle::Zero()),
      m_tri(tri)
  {}

  Object* Clone() const override{
    ObjEllipse* other = new ObjEllipse(m_tri, m_settings);
    other->SetAngleSpan(m_angleSpan);
    return other;
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    draw_ellipse_span(dc, m_tri, m_angleSpan, m_settings);
  }

  void DrawMask(FaintDC& dc, ExpressionContext&) override{
    draw_ellipse_span(dc, m_tri, m_angleSpan, mask_settings_fill(m_settings));
  }

  AngleSpan GetAngleSpan() const{
    return m_angleSpan;
  }

  coord GetArea() const override{
    if (m_angleSpan.Empty()){
      return ellipse_area(abs(get_radii(m_tri)));
    }
    else{
      return arc_area(abs(get_radii(m_tri)), m_angleSpan); // Fixme: Negative if negative span
    }
  }

  std::vector<Point> GetAttachPoints() const override{
    if (m_angleSpan.Empty()){
      return get_attach_points(m_tri);
    }
    else{
      auto pts = get_attach_points(m_tri);
      const auto endPoints = ArcEndPoints(m_tri, m_angleSpan);
      pts.push_back(endPoints.p0);
      pts.push_back(endPoints.p1);
      return pts;
    }
  }

  std::vector<Point> GetMovablePoints() const override{
    return ArcEndPoints(m_tri, m_angleSpan).GetVector();
  }

  IntRect GetRefreshRect() const override{
    return floored(bounding_rect_ink(m_tri, m_settings));
  }

  utf8_string GetType() const override{
    return "Ellipse";
  }

  int NumPoints() const override{
    return ArcEndPoints::num_points;
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return m_angleSpan.Empty() ?
      ellipse_as_path(m_tri) :
      arc_as_path(m_tri, m_angleSpan, m_settings.Get(ts_ArcSides));
  }

  Point GetPoint(int index) const override{
    assert(index < 2);
    return ArcEndPoints(m_tri, m_angleSpan)[to_size_t(index)];
  }

  Tri GetTri() const override{
    return m_tri;
  }

  void SetAngleSpan(const AngleSpan& span){
    m_angleSpan = span;
  }

  void SetPoint(const Point& p, int index) override{
    assert(index < 2);
    const auto c = center_point(m_tri);
    const auto ellipseAngle = stretch_angle(360_deg - angle360_ccw({c, p}),
      get_radii(m_tri));

    if (index == 0){
      m_angleSpan.start = ellipseAngle;
    }
    else{
      m_angleSpan.stop = ellipseAngle;
    }
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  AngleSpan m_angleSpan;
  Tri m_tri;
};

Object* create_ellipse_object_raw(const Tri& tri, const Settings& s){
  return new ObjEllipse(tri, s);
}

ObjectPtr create_ellipse_object(const Tri& tri, const Settings& s){
  return std::make_unique<ObjEllipse>(tri, s);
}

bool is_ellipse(const Object& obj){
  return is_type<ObjEllipse>(obj);
}

void set_angle_span(Object* obj, const AngleSpan& angleSpan){
  auto set_angle_span_f = [&](auto& e){ e.SetAngleSpan(angleSpan);};
  if_type<ObjEllipse>(*obj, set_angle_span_f);
}

Optional<AngleSpan> get_angle_span(const Object& obj){
  return if_type<const ObjEllipse>(obj,
    [](const ObjEllipse& e){
      return option(e.GetAngleSpan());
    },
    []() -> Optional<AngleSpan>{
      return Optional<AngleSpan>();
    });
}

} // namespace
