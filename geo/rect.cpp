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
#include "geo/point.hh"
#include "geo/rect.hh"
#include "geo/size.hh"

using std::min;
using std::max;

namespace faint{

Rect::Rect(){
  x = y = w = h = 0;
}

Rect::Rect(const std::pair<Point, Point>& pts)
  : Rect(pts.first, pts.second)
{}

Rect::Rect(const Point& p1, const Point& p2){
  x = min(p1.x, p2.x);
  y = min(p1.y, p2.y);
  w = fabs(p1.x - p2.x) + 1.0;
  h = fabs(p1.y - p2.y) + 1.0;
}

Rect::Rect(const Point& p, const Size& sz){
  x = p.x;
  y = p.y;
  w = sz.w;
  h = sz.h;
}

Size Rect::GetSize() const{
  return {w, h};
}

coord Rect::Left() const {
  return x;
}

coord Rect::Right() const {
  return x + w - 1.0;
}

coord Rect::Top() const {
  return y;
}

coord Rect::Bottom() const {
  return y + h - 1.0;
}

Point Rect::TopLeft() const {
  return {x, y};
}

Point Rect::TopRight() const {
  return {Right(), y};
}

Point Rect::BottomLeft() const{
  return {x, Bottom()};
}

Point Rect::BottomRight() const {
  return {Right(), Bottom()};
}

Point Rect::Center() const{
  return {x + (w - 1.0) / 2.0, y + (h - 1.0) / 2.0};
}

bool Rect::Contains(const Point& pt) const{
  return pt.x >= x && pt.y >= y &&
    pt.x <= Right() && pt.y <= Bottom();
}

bool empty(const Rect& r){
  return r.w == 0 || r.h == 0;
}

Rect inflated(const Rect& r, coord d){
  return inflated(r, d, d);
}

Rect inflated(const Rect& r, coord dx, coord dy){
  return {Point(r.x - dx, r.y - dy), Size(r.w + 2 * dx, r.h + 2 * dy)};
}

Rect intersection(const Rect& r1, const Rect& r2){
  const auto tl = max_coords(r1.TopLeft(), r2.TopLeft());
  const auto br = min_coords(r1.BottomRight(), r2.BottomRight());
  return br.x < tl.x || br.y < tl.y ?
    Rect() :
    Rect(tl, br);
}

bool intersects(const Rect& r1, const Rect& r2){
  return !empty(intersection(r1, r2));
}

Rect translated(const Rect& r, const Point& p){
  return {r.TopLeft() + p, r.GetSize()};
}

} // namespace
