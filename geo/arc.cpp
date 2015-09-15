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
#include <cmath>
#include "geo/angle.hh"
#include "geo/arc.hh"
#include "geo/pathpt.hh"
#include "geo/tri.hh"

namespace faint{

AngleSpan::AngleSpan()
  : start(Angle::Zero()),
    stop(Angle::Zero())
{}

AngleSpan::AngleSpan(const Angle& start, const Angle& stop)
  : start(start),
    stop(stop)
{}

int required_curve_count(const AngleSpan& angles){
  Angle span = abs(angles.Length());
  if (span <= pi / 2){
    return 2;
  }
  else if (span <= pi) {
    return 4;
  }
  else if (span <= 3 * pi / 2){
    return 6;
  }
  return 8;
}

ArcEndPoints::ArcEndPoints(const Tri& tri, const AngleSpan& angles){
  const coord rx = tri.Width() / 2;
  const coord ry = tri.Height() / 2;
  const coord aCosEta1 = rx * cos(angles.start);
  const coord bSinEta1 = ry * sin(angles.start);
  const auto mainAngle = tri.GetAngle();
  const coord cosTheta = cos(mainAngle);
  const coord sinTheta = sin(mainAngle);

  const Point c(center_point(tri));
  const coord x1 = c.x + aCosEta1 * cosTheta - bSinEta1 * sinTheta;
  const coord y1 = c.y + aCosEta1 * sinTheta + bSinEta1 * cosTheta;

  // end point
  const double aCosEta2 = rx * cos(angles.stop);
  const double bSinEta2 = ry * sin(angles.stop);
  const coord x2 = c.x + aCosEta2 * cosTheta - bSinEta2 * sinTheta;
  const coord y2 = c.y + aCosEta2 * sinTheta + bSinEta2 * cosTheta;

  p0 = Point(x1, y1);
  p1 = Point(x2, y2);
}

Point ArcEndPoints::operator[](size_t i) const{
  assert(i <= 1);
  return i == 0 ? p0 : p1;
}

std::vector<Point> ArcEndPoints::GetVector() const{
  return { p0, p1 };
}

std::vector<PathPt> arc_as_path(const Tri& tri, const AngleSpan& angles,
  bool arcSides)
{
  // This algorithm is based on documentation and code from
  // SpaceRoots.org, by L. Maisonobe "Drawing an elliptical arc using
  // polylines, quadratic or cubic Bezier curves" and
  // "EllipticalArc.java" Available here:
  // http://www.spaceroots.org/downloads.html

  const int numCurves = required_curve_count(angles);
  const double rx = tri.Width() / 2.0;
  const double ry = tri.Height() / 2.0;
  const Angle mainAngle(tri.GetAngle());
  const Point c(center_point(tri));

  Angle curveSpan = (angles.stop - angles.start) / numCurves;
  Angle currentAngle = angles.start;
  coord cosCurrentAngle = cos(currentAngle);
  coord sinCurrentAngle = sin(currentAngle);
  coord rxCosCurrentAngle = rx * cosCurrentAngle;
  coord rySinCurrentAngle = ry * sinCurrentAngle;
  coord rxSinCurrentAngle = rx * sinCurrentAngle;
  coord ryCosCurrentAngle = ry * cosCurrentAngle;
  coord sinMainAngle = sin(mainAngle);
  coord cosMainAngle = cos(mainAngle);

  coord x = c.x + rxCosCurrentAngle * cosMainAngle -
    rySinCurrentAngle * sinMainAngle;
  coord y = c.y + rxCosCurrentAngle * sinMainAngle +
    rySinCurrentAngle * cosMainAngle;
  coord xDot = -rxSinCurrentAngle * cosMainAngle -
    ryCosCurrentAngle * sinMainAngle;
  coord yDot = -rxSinCurrentAngle * sinMainAngle +
    ryCosCurrentAngle * cosMainAngle;

  std::vector<PathPt> v;
  if (arcSides){
    // Start at the tri center
    v.emplace_back(MoveTo({c.x, c.y}));

    // Arc start point
    v.emplace_back(LineTo({x, y}));
  }
  else{
    v.emplace_back(MoveTo({x, y}));

  }
  coord t = tan(0.5 * curveSpan);
  coord alpha = sin(curveSpan) * (std::sqrt(4 + 3 * t * t) - 1) / 3.0;

  for (int i = 0; i != numCurves; i++){
    coord prevX = x;
    coord prevY = y;
    double prevXDot = xDot;
    double prevYDot = yDot;
    currentAngle += curveSpan;
    cosCurrentAngle = cos(currentAngle);
    sinCurrentAngle = sin(currentAngle);
    rxCosCurrentAngle = rx * cosCurrentAngle;
    rySinCurrentAngle = ry * sinCurrentAngle;
    rxSinCurrentAngle = rx * sinCurrentAngle;
    ryCosCurrentAngle = ry * cosCurrentAngle;
    x = c.x + rxCosCurrentAngle * cosMainAngle - rySinCurrentAngle * sinMainAngle;
    y = c.y + rxCosCurrentAngle * sinMainAngle + rySinCurrentAngle * cosMainAngle;
    xDot = -rxSinCurrentAngle * cosMainAngle - ryCosCurrentAngle * sinMainAngle;
    yDot = -rxSinCurrentAngle * sinMainAngle + ryCosCurrentAngle * cosMainAngle;

    v.emplace_back(CubicBezier({x,y},
        {prevX + alpha * prevXDot, prevY + alpha * prevYDot},
        {x - alpha * xDot, y - alpha * yDot}));
  }
  if (arcSides){
    v.emplace_back(Close());
  }
  return v;
}

coord arc_area(const Radii& r, const AngleSpan& angles){
  // Based on "The Area of Intersecting Ellipses" by Dave Eberly
  // http://www.geometrictools.com/Documentation/AreaIntersectingEllipses.pdf

  auto F = [a=r.x, b=r.y](coord theta){
    return ((a*b)/2) *
    (theta - std::atan(
      ((b - a)*std::sin(2*theta)) /
      (b + a + (b - a)*std::cos(2*theta))));
  };

  return F(angles.stop.Rad()) - F(angles.start.Rad());
}

coord circle_arc_area(coord r, const Angle& a){
  return sq(r) * a.Rad() / 2;
}

} // namespace
