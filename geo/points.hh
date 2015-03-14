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

#ifndef FAINT_POINTS_HH
#define FAINT_POINTS_HH
#include <vector>
#include "geo/pathpt.hh"
#include "geo/tri.hh"

namespace faint{

// Simplistic, returns a tri for the points, considering
// only x, y values
Tri tri_from_points(const std::vector<PathPt>&);
Tri tri_from_points(const std::vector<Point>&);

class Points {
public:
  Points();
  Points(const Points&);
  explicit Points(const std::vector<PathPt>&);
  void AdjustBack(const PathPt&);
  void AdjustBack(const Point&);
  void Append(const PathPt&);
  void Append(const Point&);
  const PathPt& Back() const;
  const PathPt& BaBack() const;
  void Clear();
  bool Empty() const;
  const PathPt& Front() const;
  std::vector<PathPt> GetPoints(const Tri&) const;
  std::vector<PathPt> GetPoints() const;
  std::vector<Point> GetPointsDumb() const;
  std::vector<Point> GetPointsDumb(const Tri&) const;
  Tri GetTri() const;
  void InsertPoint(const Tri&, const Point&, int index);
  PathPt PopBack();
  void RemovePoint(const Tri&, int index);
  void SetPoint(const Tri&, const Point&, int index);
  void SetPoint(const Tri&, const PathPt&, int index);
  void SetTri(const Tri&);
  int Size() const;
private:
  Tri m_tri;
  std::vector<PathPt> m_points;
  mutable std::vector<PathPt> m_cache;
  mutable Tri m_cacheTri;
};

// Returns a Points object initialized with the coordinates
// interpreted as x,y-pairs.
// Asserts that the number of coordinates is even.
Points points_from_coords(const std::vector<coord>&);

std::vector<PathPt> to_line_path(const std::vector<Point>&);


} // namespace

#endif
