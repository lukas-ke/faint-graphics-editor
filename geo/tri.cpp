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

#include <cmath>
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/radii.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "util/optional.hh"

namespace faint{

Adj::Adj()
  : scale_x(1.0),
    scale_y(1.0),
    tr_x(0.0),
    tr_y(0.0),
    angle(Angle::Zero()),
    skew(0.0)
{}

coord delta_from_length(coord length){
  return length < 0 ? (length + 1) : (length - 1);
}

Tri::Tri(const Point& p0, const Point& p1, coord h)
  : m_p0(p0),
    m_p1(p1)
{
  Angle angle = line_angle({p0, p1});
  coord dy = delta_from_length(h);
  m_p2 = m_p0 + Point(dy * cos(angle + pi / 2), dy * sin(angle + pi / 2));
}

Tri::Tri(const Point& p0, const Angle& angle, const Size& sz)
  : m_p0(p0)
{
  coord dx = delta_from_length(sz.w);
  coord dy = delta_from_length(sz.h);
  m_p1 = m_p0 + Point(dx * cos(angle), dx * sin(angle));
  m_p2 = m_p0 + Point(dy * cos(angle + pi / 2), dy * sin(angle + pi / 2));
}

Tri::Tri(const Point& p0, const Point& p1, const Point& p2)
  : m_p0(p0),
    m_p1(p1),
    m_p2(p2)
{}

Angle Tri::GetAngle() const {
  return line_angle({m_p0, m_p1});
}

coord Tri::Width() const {
  return distance(m_p0, m_p1);
}

coord Tri::Height() const {
  Angle angle1 = line_angle({m_p0, m_p1});
  Angle angle2 = line_angle({m_p0, m_p2});
  Angle angle3 = angle1 + pi / 2.0;
  coord h = distance(m_p0, m_p2);

  // Fixme: Seems nutty.
  if (std::fabs(sin(angle2) - sin(angle3)) > 0.1){
    return -h;
  }
  else if (fabs(cos(angle2) - cos(angle3)) > 0.1){
    return -h;
  }
  return h;
}

coord Tri::Skew() const{
  // Fixme: Is -GetAngle correct (considering normalization etc)
  Point p0 = rotate_point(m_p0, -GetAngle(), m_p2);
  return p0.x - m_p2.x;
}

bool Tri::operator==(const Tri& other) const{
  return m_p0 == other.m_p0 && m_p1 == other.m_p1 && m_p2 == other.m_p2;
}

Point Tri::P0() const {
  return m_p0;
}

Point Tri::P1() const {
  return m_p1;
}

Point Tri::P2() const {
  return m_p2;
}

Point Tri::P3() const {
  const coord dx = Width();
  const Angle angle(GetAngle());
  return m_p2 + Point(dx * cos(angle), dx * sin(angle));
}

bool Tri::Contains(const Point& pt) const{
  Rect r(union_of(Rect(m_p0, P1()), Rect(P1(), P2())));
  return r.Contains(pt);
}

coord area(const Tri& tri){
  Tri t2 = rotated(tri, -tri.GetAngle(), tri.P0());
  coord h = std::abs(t2.P0().y - t2.P3().y);
  return tri.Width() * h;
}

Rect bounding_rect(const Tri& tri){
  Point p0 = tri.P0();
  Point p1 = tri.P1();
  Point p2 = tri.P2();
  Point p3 = tri.P3();
  Point minPt = min_coords(p0, p1, p2, p3);
  Point maxPt = max_coords(p0, p1, p2, p3);
  return Rect(minPt, maxPt);
}

Point center_point(const Tri& t){
  return intersection(unbounded(LineSegment(t.P0(), t.P3())),
    unbounded(LineSegment(t.P1(), t.P2()))).Visit(
      [](const Point& center){
        return center;
      },
      [&t](){
        return t.P0();
      });
}

Adj get_adj(const Tri& t1, const Tri& t2){
  Adj adj;

  // De-skew
  Tri t1b = skewed(t1, -t1.Skew());
  Tri t2b = skewed(t2, -t2.Skew());
  adj.scale_x = t1b.Width() == 0 ? 1 :
    t2b.Width() / t1b.Width();
  adj.scale_y = t1b.Height() == 0 ? 1 :
    t2b.Height() / t1b.Height();
  adj.angle = t2b.GetAngle() - t1b.GetAngle();
  adj.tr_x = t2b.P3().x - t1b.P3().x;
  adj.tr_y = t2b.P3().y - t1b.P3().y;
  adj.skew = t2.Skew() - t1.Skew();
  return adj;
}

Radii get_radii(const Tri& tri){
  return Radii(tri.Width() / 2, tri.Height() / 2);
}

Point mid_P0_P1(const Tri& t){
  return t.P0() + Point((t.Width() / 2.0) * cos(t.GetAngle()),
    (t.Width() / 2.0) * sin(t.GetAngle()));

}
Point mid_P0_P2(const Tri& t){
  return t.P0() + Point((t.Height())/2.0 * cos(t.GetAngle() + pi / 2.0),
    (t.Height() / 2.0) * sin(t.GetAngle() + pi / 2.0));
}

Point mid_P1_P3(const Tri& t){
  return t.P1() + Point((t.Height()/2.0) * cos(t.GetAngle() + pi / 2.0),
    (t.Height()/2.0) * sin(t.GetAngle() + pi / 2.0));
}

Point mid_P2_P3(const Tri& t){
  return t.P2() + Point((t.Width() / 2.0) * cos(t.GetAngle()),
    (t.Width() / 2.0) * sin(t.GetAngle()));
}


static Tri offset_P0_P1(const Tri& t, coord xOff){
  // Fixme: Merge offset_P0_* into offset_aligned
  Angle a = line_angle(LineSegment(t.P0(), t.P1()));

  coord dx = xOff * cos(a);
  coord dy = xOff * sin(a);

  return translated(t, dx, dy);
}

static Tri offset_P0_P2(const Tri& t, coord yOff){
  // Fixme: Merge offset_P0_* into offset_aligned
  Angle a = line_angle(LineSegment(t.P0(), t.P2()));

  coord dx = yOff * cos(a);
  coord dy = yOff * sin(a);

  return translated(t, dx, dy);
}

Tri offset_aligned(const Tri& t, coord xOff, coord yOff){
  return offset_P0_P2(offset_P0_P1(t, xOff), yOff);
}

Tri rotated(const Tri& t0, const Angle& angle, Point origin){
  Point p0 = rotate_point(t0.P0(), angle, origin);
  Point p1 = rotate_point(t0.P1(), angle, origin);
  Point p2 = rotate_point(t0.P2(), angle, origin);
  return Tri(p0, p1, p2);
}

Tri scaled(const Tri& t0, const Scale& scale, const Point& origin){
  Point p0 = scale_point(t0.P0(), scale, origin);
  Point p1 = scale_point(t0.P1(), scale, origin);
  Point p2 = scale_point(t0.P2(), scale, origin);
  return Tri(p0, p1, p2);
}

Tri skewed(const Tri& t, coord skewX){
  Point p0 = t.P0();
  Point p1 = t.P1();
  Point p2 = t.P2();
  Point p3 = t.P3();

  p0 = rotate_point(p0, -t.GetAngle(), p2);
  p1 = rotate_point(p1, -t.GetAngle(), p3);
  p0.x += skewX;
  p1.x += skewX;
  p0 = rotate_point(p0, t.GetAngle(), p2);
  p1 = rotate_point(p1, t.GetAngle(), p3);
  return Tri(p0, p1, p2);
}

Tri translated(const Tri& t, coord tX, coord tY){
  Point off(tX, tY);
  return Tri(t.P0() + off, t.P1() + off, t.P2() + off);
}

Tri tri_from_rect(const Rect& r){
  return Tri(r.TopLeft(), r.TopRight(), r.BottomLeft());
}

bool valid(const Tri& t){
  return valid(t.m_p0) &&
    valid(t.m_p1) &&
    valid(t.m_p2);
}

bool rather_zero(const Tri& t){
  return rather_zero(t.Width()) || rather_zero(t.Height());
}

LineSegment P0_P1(const Tri& t){
  return LineSegment(t.P0(), t.P1());
}

LineSegment P0_P2(const Tri& t){
  return LineSegment(t.P0(), t.P2());
}

} // namespace
