// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include <cmath> // fabs
#include "geo/adjust.hh"
#include "geo/angle.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/point.hh"

namespace faint{

Point adjust_to(const Point& o,
  const Point& p,
  const Angle& step,
  const Angle& offset)
{
  const int n_angles = 360 / truncated(step.Deg()) + 1;
  const coord length = distance(o, p);
  const Angle angle = angle360({o, p});

  for (int i = 0; i!= n_angles; i++){
    Angle i_angle = i * step + offset;
    if (abs(angle - i_angle) <= step / 2.0){
      Angle newAngle = tau - i_angle;
      coord x_mplr = cos(newAngle);
      coord y_mplr = sin(newAngle);
      coord x = x_mplr * length + o.x;
      coord y = y_mplr * length + o.y;
      return Point(x, y);
    }
  }
  return p;
}

Point adjust_to_45(const Point& o, const Point& p){
  const Angle angle = angle360({o, p});
  coord dx = p.x - o.x;
  coord dy = p.y - o.y;

  if (Angle::Deg(360 - 22.5) < angle || angle <= Angle::Deg(22.5)){
    return Point(o.x + dx, o.y);
  }
  else if (Angle::Deg(22.5) < angle && angle <= Angle::Deg(67.5)){
    return Point(o.x - dy, o.y + dy);
  }
  else if (Angle::Deg(67.5) < angle && angle <= Angle::Deg(110.5)){
    return Point(o.x, o.y + dy);
  }
  else if (Angle::Deg(110.5) < angle && angle <= Angle::Deg(157.5)){
    return Point(o.x + dy, o.y + dy);
  }
  else if (Angle::Deg(157.5) < angle && angle <= Angle::Deg(202.5)){
    return Point(o.x + dx, o.y);
  }
  else if (Angle::Deg(202.5) < angle && angle <= Angle::Deg(247.5)){
    return Point(o.x - dy, o.y + dy);
  }
  else if (Angle::Deg(247.5) < angle && angle <= Angle::Deg(292.5)){
    return Point(o.x, o.y + dy);
  }
  else if (Angle::Deg(292.5) < angle && angle < Angle::Deg(360 - 22.5)){
    return Point(o.x + dy, o.y + dy);
  }
  return p;
}

Point adjust_to_default(const Point& origin,
  const Point& p,
  const Angle& angle,
  const Angle& altAngle)
{
  auto twoPi = 2 * pi.Rad();
  int n_angles = static_cast<int>(twoPi / angle.Rad()) + 1;
  auto length = distance(origin, p);
  auto initialAngle = -line_angle({origin, p}).Rad();
  if (initialAngle < 0){
    initialAngle = twoPi + initialAngle;
  }

  auto foundAngle = initialAngle;
  for (int i = 0; i!= n_angles; i++){
    auto thisAngle = i * angle.Rad();
    if (std::fabs(initialAngle - thisAngle) <= angle.Rad() / 2){
      foundAngle = thisAngle;
      break;
    }
  }

  if (std::fabs(initialAngle - altAngle.Rad()) <
    std::fabs(initialAngle - foundAngle))
  {
    // Use the altAngle instead
    foundAngle = altAngle.Rad();
  }

  auto newAngle = -foundAngle;
  coord x_mplr = std::cos(newAngle);
  coord y_mplr = std::sin(newAngle);
  coord x = x_mplr * length + origin.x;
  coord y = y_mplr * length + origin.y;
  return Point(x, y);
}

ConstrainDir constrain_pos(IntPoint& p, const IntPoint& origin){
  if (std::abs(origin.x - p.x) > std::abs(origin.y - p.y)){
    p.y = origin.y;
    return ConstrainDir::HORIZONTAL;
  }
  else {
    p.x = origin.x;
    return ConstrainDir::VERTICAL;
  }
}

ConstrainDir constrain_pos(Point& p, const Point& origin){
  if (std::fabs(origin.x - p.x) > std::fabs(origin.y - p.y)){
    p.y = origin.y;
    return ConstrainDir::HORIZONTAL;
  }
  else {
    p.x = origin.x;
    return ConstrainDir::VERTICAL;
  }
}

void constrain_pos(Point& p, const Point& origin, ConstrainDir dir){
  if (dir == ConstrainDir::HORIZONTAL){
    p.y = origin.y;
  }
  else if (dir == ConstrainDir::VERTICAL){
    p.x = origin.x;
  }
}

void constrain_pos(IntPoint& p, const IntPoint& origin, ConstrainDir dir){
  if (dir == ConstrainDir::HORIZONTAL){
    p.y = origin.y;
  }
  else if (dir == ConstrainDir::VERTICAL){
    p.x = origin.x;
  }
}

Point constrain_proportional(const Point& moved,
  const Point& opposite,
  const Point& origin)
{
  // Moved point must lie on the diagonal between opposite and origin
  coord dx = origin.x - opposite.x;
  coord dy = origin.y - opposite.y;
  coord sign = (moved.x < 0) == (moved.y < 0) ? 1.0 : -1.0;
  coord x2 = sign * ((dx/dy) * moved.y + origin.x - (dx/dy) * origin.y);
  coord y2 = sign * ((dy/dx) * moved.x + origin.y - (dy/dx) * origin.x);

  return fabs(x2) > fabs(moved.x) ?
    Point(x2, moved.y) :
    Point(moved.x, y2);
}

} // namespace
