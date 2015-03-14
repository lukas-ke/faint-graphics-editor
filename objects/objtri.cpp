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
#include "geo/pathpt.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objtri.hh"
#include "rendering/faint-dc.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"
#include "util-wx/font.hh"

namespace faint{

static Settings tri_text_settings(){
  Settings s;
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_Bg, Paint(Color(255, 255, 255)));
  s.Set(ts_SwapColors, false);
  s.Set(ts_FontSize, get_default_font_size());
  s.Set(ts_FontFace, get_default_font_name());
  s.Set(ts_FontBold, false);
  s.Set(ts_FontItalic, false);
  return s;
}

class ObjTri : public Object{
public:
  ObjTri(const Tri& tri, const Settings& s)
    : Object(s),
      m_tri(tri)
  {}

  Object* Clone() const override{
    return new ObjTri(m_tri, GetSettings());
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    Tri tri(m_tri);
    Point p0(tri.P0());
    Point p1(tri.P1());
    Point p2(tri.P2());
    Point p3(tri.P3());
    dc.PolyLine(tri, {p0, p1}, m_settings);
    dc.PolyLine(tri, {p0, p2}, m_settings);
    Settings txtSettings(tri_text_settings());

    auto point_label = [&](const Point& p, const utf8_string& s){
      Rect r(p, Size(100, 20));
      Tri tri(tri_from_rect(r));
      dc.Text(tri, s, txtSettings);
    };

    point_label(p0, "p0=");
    point_label(p1, "p1=");
    point_label(p2, "p2=");
    point_label(p3, "p3=");

    Rect r(bounding_rect(tri));
    dc.Text(tri_from_rect(Rect(r.TopRight() + Point(10,0), Size(100,100))),
      utf8_string("w="),
      txtSettings);
  }

  void DrawMask(FaintDC&) override{
  }

  std::vector<Point> GetAttachPoints() const override{
    return get_attach_points(m_tri);
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return {PathPt::MoveTo(m_tri.P0()),
        PathPt::LineTo(m_tri.P1()),
        PathPt::MoveTo(m_tri.P1()),
        PathPt::LineTo(m_tri.P3())};
  }

  IntRect GetRefreshRect() const override{
    return floored(inflated(bounding_rect(m_tri), 200));
  }

  Tri GetTri() const override{
    return m_tri;
  }

  utf8_string GetType() const override{
    return "TriObject";
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  Tri m_tri;
};

Object* create_tri_object(const Tri& t, const Settings& s){
  return new ObjTri(t, s);
}

} // namespace
