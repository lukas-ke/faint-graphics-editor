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
  const Angle angle = angle360_ccw({o, p});

  for (int i = 0; i!= n_angles; i++){
    Angle i_angle = i * step + offset;
    if (abs(angle - i_angle) <= step / 2.0){
      Angle newAngle = tau - i_angle;
      coord x_mplr = cos(newAngle);
      coord y_mplr = sin(newAngle);
      coord x = x_mplr * length + o.x;
      coord y = y_mplr * length + o.y;
      return {x, y};
    }
  }
  return p;
}

Point adjust_to_45(const Point& o, const Point& p){
  const auto angle = angle360_ccw({o, p});
  const auto dx = p.x - o.x;
  const auto dy = p.y - o.y;

  if (360_deg - 22.5_deg < angle || angle <= 22.5_deg){
    return {o.x + dx, o.y};
  }
  else if (22.5_deg < angle && angle <= 67.5_deg){
    return {o.x - dy, o.y + dy};
  }
  else if (67.5_deg < angle && angle <= 110.5_deg){
    return {o.x, o.y + dy};
  }
  else if (110.5_deg < angle && angle <= 157.5_deg){
    return {o.x + dy, o.y + dy};
  }
  else if (157.5_deg < angle && angle <= 202.5_deg){
    return {o.x + dx, o.y};
  }
  else if (202.5_deg < angle && angle <= 247.5_deg){
    return {o.x - dy, o.y + dy};
  }
  else if (247.5_deg < angle && angle <= 292.5_deg){
    return {o.x, o.y + dy};
  }
  else if (292.5_deg < angle && angle < 360_deg - 22.5_deg){
    return {o.x + dy, o.y + dy};
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
  auto initialAngle = -line_angle_cw({origin, p}).Rad();
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

  const auto newAngle = -foundAngle;
  const coord x_mplr = std::cos(newAngle);
  const coord y_mplr = std::sin(newAngle);
  const coord x = x_mplr * length + origin.x;
  const coord y = y_mplr * length + origin.y;
  return {x, y};
}

ConstrainDir constrain_pos(IntPoint& p, const IntPoint& origin){
  if (std::abs(origin.x - p.x) > std::abs(origin.y - p.y)){
    p.y = origin.y;
    return ConstrainDir::HORIZONTAL;
  }
  else{
    p.x = origin.x;
    return ConstrainDir::VERTICAL;
  }
}

ConstrainDir constrain_pos(Point& p, const Point& origin){
  if (std::fabs(origin.x - p.x) > std::fabs(origin.y - p.y)){
    p.y = origin.y;
    return ConstrainDir::HORIZONTAL;
  }
  else{
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
