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

namespace faint{

static void draw_ellipse_span(FaintDC& dc,
  const Tri& tri,
  const AngleSpan& angles,
  const Settings& settings)
{
  if (angles.start == angles.stop){
    dc.Ellipse(tri, settings);
  }
  else {
    dc.Arc(tri, angles, settings);
  }
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

  std::vector<Point> GetAttachPoints() const override{
    if (m_angleSpan.start == m_angleSpan.stop){
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
    return floored(bounding_rect(m_tri, m_settings));
  }

  utf8_string GetType() const override{
    return "Ellipse";
  }

  int NumPoints() const override{
    return ArcEndPoints::num_points;
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    if (m_angleSpan.start == m_angleSpan.stop){
      return ellipse_as_path(m_tri);
    }
    else{
      Point c = center_point(m_tri);
      Radii r = get_radii(m_tri);
      Point start = c + polar(r, m_angleSpan.start);
      Point stop = c + polar(r, m_angleSpan.stop);

      // Fixme: Compute arc-flag/sweep-flag
      // Fixme: Consider ts_ArcSides
      return {
        PathPt::MoveTo(c),
        PathPt::LineTo(start),
        PathPt::Arc(r, 0_rad, 1, 1, stop),
        PathPt::PathCloser()};
    }
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
    Point c(center_point(m_tri));
    // Fixme: The angle-span will be different after
    // \def(elliptic-arc-problem);
    // e.SetPoint(<whatever>, 0)
    // followed by
    // e.SetPoint(e.GetPoint(0), 0),
    // because ArcEndPoints and angle360_ccw({c, p}) disagree increasingly
    // the less circular we are.
    if (index == 0){
      m_angleSpan.start = 360_deg - angle360_ccw({c, p});
    }
    else{
      m_angleSpan.stop = 360_deg - angle360_ccw({c, p});
    }
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  AngleSpan m_angleSpan;
  Tri m_tri;
};

Object* create_ellipse_object(const Tri& tri, const Settings& s){
  return new ObjEllipse(tri, s);
}

bool is_ellipse(const Object* obj){
  return dynamic_cast<const ObjEllipse*>(obj) != nullptr;
}

void set_angle_span(Object* obj, const AngleSpan& angleSpan){
  if (ObjEllipse* ellipse = dynamic_cast<ObjEllipse*>(obj)){
    ellipse->SetAngleSpan(angleSpan);
  }
}

Optional<AngleSpan> get_angle_span(const Object* obj){
  if (const ObjEllipse* ellipse = dynamic_cast<const ObjEllipse*>(obj)){
    return option(ellipse->GetAngleSpan());
  }
  return no_option();
}

} // namespace
