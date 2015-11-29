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

#include <algorithm> // min, max
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"

namespace faint{

IntRect::IntRect(){
  x = y = w = h = 0;
}

IntRect::IntRect(const IntPoint& p0, const IntPoint& p1) :
  x(std::min(p0.x, p1.x)),
  y(std::min(p0.y, p1.y)),
  w(std::abs(p0.x - p1.x) + 1),
  h(std::abs(p0.y - p1.y) + 1)
{}

IntRect::IntRect(const IntPoint& pt, const IntSize& sz)
  : x(pt.x),
    y(pt.y),
    w(sz.w),
    h(sz.h)
{}

IntRect IntRect::EmptyRect(){
  return {};
}

IntSize IntRect::GetSize() const{
  return {w, h};
}

int IntRect::Left() const{
  return x;
}

int IntRect::Right() const{
  return x + w - 1;
}

int IntRect::Top() const{
  return y;
}

int IntRect::Bottom() const{
  return y + h - 1;
}

IntPoint IntRect::TopLeft() const{
  return {x,y};
}

IntPoint IntRect::TopRight() const{
  return {Right(),y};
}

IntPoint IntRect::BottomLeft() const{
  return {x, Bottom()};
}

IntPoint IntRect::BottomRight() const{
  return {Right(), Bottom()};
}

IntPoint IntRect::MidTop() const{
  return {x + w / 2, Top()};
}

IntPoint IntRect::MidBottom() const{
  return {x + w / 2, Bottom()};
}

IntPoint IntRect::MidLeft() const{
  return {Left(), y + h / 2};
}

IntPoint IntRect::MidRight() const{
  return {Right(), y + h / 2};
}

bool IntRect::Contains(const IntPoint& p) const{
  return
    p.x >= x && p.y >= y &&
    p.x <= Right() && p.y <= Bottom();
}

void IntRect::MoveTo(const IntPoint& p){
  x = p.x;
  y = p.y;
}

int area(const IntRect& r){
  return r.w * r.h;
}

bool empty(const IntRect& r){
  return r.w == 0 || r.h == 0;
}

IntRect inflated(const IntRect& r, int dx, int dy){
  return {IntPoint(r.x - dx, r.y - dy),
    IntSize(r.w + 2 * dx, r.h + 2 * dy)};
}

IntRect inflated(const IntRect& r, int d){
  return inflated(r, d, d);
}

IntRect deflated(const IntRect& r, int d){
  return {IntPoint(r.x + d, r.y + d),
    IntSize(r.w - 2 * d, r.h - 2 * d)};
}

IntRect intersection(const IntRect& r1, const IntRect& r2){
  const auto tl = max_coords(r1.TopLeft(), r2.TopLeft());
  const auto br = min_coords(r1.BottomRight(), r2.BottomRight());
  return br.x < tl.x || br.y < tl.y ?
    IntRect::EmptyRect() :
    IntRect(tl, br);
}

IntRect largest(const IntRect& r1, const IntRect& r2){
  return area(r1) <= area(r2) ? r2 : r1;
}

IntRect smallest(const IntRect& r1, const IntRect& r2){
  return area(r2) < area(r1) ? r2 : r1;
}

IntRect translated(const IntRect& r, const IntPoint& p){
  return {IntPoint(r.x + p.x, r.y + p.y), IntSize(r.w, r.h)};
}

bool operator==(const IntRect& r1, const IntRect& r2){
  return
    r1.x == r2.x &&
    r1.y == r2.y &&
    r1.w == r2.w &&
    r1.h == r2.h;
}

bool operator!=(const IntRect& r1, const IntRect& r2){
  return !(r1 == r2);
}

} // namespace
