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
  return Size(w, h);
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
  return Point(x, y);
}

Point Rect::TopRight() const {
  return Point(Right(), y);
}

Point Rect::BottomLeft() const{
  return Point(x, Bottom());
}

Point Rect::BottomRight() const {
  return Point(Right(), Bottom());
}

Point Rect::Center() const{
  return Point(x + (w - 1.0) / 2.0, y + (h - 1.0) / 2.0);
}

bool Rect::Contains(const Point& pt) const{
  return pt.x >= x && pt.y >= y && pt.x <= Right() && pt.y <= Bottom();
}

bool empty(const Rect& r){
  return r.w == 0 || r.h == 0;
}

Rect inflated(const Rect& r, coord d){
  return inflated(r, d, d);
}

Rect inflated(const Rect& r, coord dx, coord dy){
  return Rect(Point(r.x - dx, r.y - dy), Size(r.w + 2 * dx, r.h + 2 * dy));
}

Rect intersection(const Rect& r1, const Rect& r2){
  coord x1 = r1.x < r2.x ? r2.x : r1.x;
  coord y1 = r1.y < r2.y ? r2.y : r1.y;
  coord x2 = r1.Right();
  coord y2 = r1.Bottom();

  if (x2 > r2.Right()){
    x2 = r2.Right();
  }
  if (y2 > r2.Bottom()){
    y2 = r2.Bottom();
  }

  coord w2 = x2 - x1 + 1.0;
  coord h2 = y2 - y1 + 1.0;
  if (w2 <= 0 || h2 <= 0){
    return Rect();
  }
  return Rect(Point(x1, y1), Size(w2, h2));
}

bool intersects(const Rect& r1, const Rect& r2){
  coord x1 = r1.x < r2.x ? r2.x : r1.x;
  coord y1 = r1.y < r2.y ? r2.y : r1.y;
  coord x2 = r1.Right();
  coord y2 = r1.Bottom();

  if (x2 > r2.Right()){
    x2 = r2.Right();
  }
  if (y2 > r2.Bottom()){
    y2 = r2.Bottom();
  }

  coord w2 = x2 - x1 + 1.0;
  coord h2 = y2 - y1 + 1.0;
  if (w2 < 0 || h2 < 0){
    return false;
  }
  return true;
}

Rect translated(const Rect& r, const Point& p){
  return Rect(r.TopLeft() + p, r.GetSize());
}

Rect union_of(const Rect& r1, const Rect& r2) {
  const coord left = min(r1.x, r2.x);
  const coord top = min(r1.y, r2.y);
  const coord bottom = max(r1.Bottom(), r2.Bottom());
  const coord right = max(r1.Right(), r2.Right());
  return Rect(Point(left, top), Point(right, bottom));
}

} // namespace
