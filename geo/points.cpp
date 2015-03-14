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
#include <cassert>
#include "geo/geo-func.hh"
#include "geo/points.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "util/iter.hh"

namespace faint{

Tri tri_from_points(const std::vector<PathPt>& points){
  if (points.empty()){
    return Tri(Point(0,0), Angle::Zero(), Size(0,0));
  }
  Point topLeft = points[0].p;
  Point bottomRight = topLeft;

  for (const PathPt& pt : points){
    if (pt.ClosesPath()){
      continue;
    }
    topLeft = min_coords(topLeft, pt.p);
    bottomRight = max_coords(bottomRight, pt.p);
    if (pt.IsCubicBezier()){
      // Fixme: The bezier is always bounded by the end points and control
      // points, but this isn't the tightest boundary.
      topLeft = min_coords(topLeft, pt.c, pt.d);
      bottomRight = max_coords(bottomRight, pt.c, pt.d);
    }
  }
  return Tri(topLeft,
    Point(bottomRight.x, topLeft.y),
    Point(topLeft.x, bottomRight.y));
}

Tri tri_from_points(const std::vector<Point>& points){
  if (points.empty()){
    return Tri(Point(0,0), Angle::Zero(), Size(0,0));
  }
  const Point& p0 = points[0];
  coord x_min = p0.x;
  coord x_max = p0.x;
  coord y_min = p0.y;
  coord y_max = p0.y;

  for (size_t i = 0; i != points.size(); i++){
    const Point& pt = points[i];
    x_min = std::min(x_min, pt.x);
    x_max = std::max(x_max, pt.x);
    y_min = std::min(y_min, pt.y);
    y_max = std::max(y_max, pt.y);
  }
  return Tri(Point(x_min, y_min), Point(x_max, y_min), Point(x_min, y_max));
}

Points::Points()
  : m_tri(Point(0,0), Angle::Zero(), faint::Size(0,0)),
    m_cacheTri(m_tri)
{}

Points::Points(const Points& other)
  : m_tri(other.m_tri),
    m_points(other.m_points),
    m_cache(),
    m_cacheTri()
{}

Points::Points(const std::vector<PathPt>& points) :
  m_tri(tri_from_points(points)),
  m_points(points),
  m_cache(),
  m_cacheTri()
{}

void Points::AdjustBack(const PathPt& pt){
  PathPt& back(m_points.back());
  back = pt;
  m_tri = tri_from_points(m_points);
}

void Points::AdjustBack(const Point& p){
  PathPt& back(m_points.back());
  back.p = p;
  m_tri = tri_from_points(m_points);
}

void Points::Append(const PathPt& p){
  m_points.push_back(p);
  m_tri = tri_from_points(m_points);
}

void Points::Append(const Point& p){
  if (m_points.empty()){
    m_points.push_back(PathPt::MoveTo(p));
  }
  else{
    m_points.push_back(PathPt::LineTo(p));
  }
  m_tri = tri_from_points(m_points);
}

const PathPt& Points::Back() const{
  return m_points.back();
}

const PathPt& Points::BaBack() const{
  assert(m_points.size() >= 2);
  return m_points[m_points.size() - 2];
}

void Points::Clear(){
  m_points.clear();
  m_tri = Tri(Point(0,0), Angle::Zero(), faint::Size(0,0));
}

bool Points::Empty() const{
  return m_points.empty();
}

const PathPt& Points::Front() const{
  return m_points.front();
}

std::vector<PathPt> Points::GetPoints(const Tri& tri) const{
  if (m_cache.size() == m_points.size() && m_cacheTri == tri){
    return m_cache;
  }

  if (m_cache.size() != m_points.size()){
    m_cache = m_points;
  }
  Adj a = get_adj(m_tri, tri);
  coord p2y = m_tri.P2().y;
  coord ht = m_tri.Height();
  for (size_t i = 0; i != m_points.size(); i++){
    PathPt pt = m_points[i];
    if (ht != 0){
      pt.p.x += a.skew * ((p2y - pt.p.y) / ht) / a.scale_x; // assumes m_tri not rotated
    }
    pt.p.x += a.tr_x;
    pt.p.y += a.tr_y;

    if (ht != 0){
      pt.c.x += a.skew * ((p2y - pt.c.y) / ht) / a.scale_x; // assumes m_tri not rotated
    }
    pt.c.x += a.tr_x;
    pt.c.y += a.tr_y;

    if (ht != 0){
      pt.d.x += a.skew * ((p2y - pt.d.y) / ht) / a.scale_x; // assumes m_tri not rotated
    }
    pt.d.x += a.tr_x;
    pt.d.y += a.tr_y;

    pt = scale_point(pt, Scale(a.scale_x, a.scale_y), tri.P3());
    pt = rotate_point(pt, tri.GetAngle(), tri.P3());

    m_cache[i] = pt;
  }
  return m_cache;
}

std::vector<PathPt> Points::GetPoints() const{
  return m_points;
}

std::vector<Point> Points::GetPointsDumb() const{
  std::vector<Point> v;
  for (size_t i = 0; i != m_points.size(); i++){
    const PathPt& pt(m_points[i]);
    v.push_back(pt.p);
  }
  return v;
}

std::vector<Point> Points::GetPointsDumb(const Tri& tri) const{
  std::vector<PathPt> pts(GetPoints(tri));
  std::vector<Point> v;
  for (size_t i = 0; i != m_points.size(); i++){
    PathPt& pt(pts[i]);
    v.push_back(pt.p);
  }
  return v;
}

Tri Points::GetTri() const {
  return m_tri;
}

void Points::InsertPoint(const Tri& tri, const Point& p, int index){
  assert(index >= 0);
  m_points = GetPoints(tri);
  m_points.insert(begin(m_points) + index, PathPt::LineTo(p));
  m_tri = tri_from_points(m_points);
}

PathPt Points::PopBack(){
  PathPt p = m_points.back();
  m_points.pop_back();
  m_tri = tri_from_points(m_points);
  return p;
}

void Points::RemovePoint(const Tri& tri, int index){
  m_points = GetPoints(tri);
  m_points.erase(begin(m_points) + index);
  m_tri = tri_from_points(m_points);
}

void Points::SetPoint(const Tri& tri, const Point& p, int index){
  m_points = GetPoints(tri);
  m_points[to_size_t(index)].p = p;
  m_tri = tri_from_points(m_points);
}

void Points::SetPoint(const Tri& tri, const PathPt& p, int index){
  m_points = GetPoints(tri);
  m_points[to_size_t(index)] = p;
  m_tri = tri_from_points(m_points);
}

void Points::SetTri(const Tri& tri){
  m_tri = tri;
}

int Points::Size() const{
  return resigned(m_points.size());
}

Points points_from_coords(const std::vector<coord>& coords){
  const size_t n = coords.size();
  assert(n % 2 == 0);
  Points points;
  for (size_t i = 0; i != n; i+=2){
    coord x = coords[i];
    coord y = coords[i +1];
    points.Append(Point(x,y));
  }
  return points;
}

std::vector<PathPt> to_line_path(const std::vector<Point>& points){
  assert(points.size() > 1);
  std::vector<PathPt> path;
  path.push_back(PathPt::MoveTo(points.front()));
  for (const Point& p : but_first(points)){
    path.push_back(PathPt::LineTo(p));
  }
  return path;
}

} // namespace
