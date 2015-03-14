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

#include <vector>
#include "bitmap/alpha-map.hh"
#include "bitmap/bitmap-fwd.hh"

namespace faint{

class Bitmap;
class Brush;
class IntLineSegment;
class Point;

// Returns a vector of lines following the outline of the Brush, in
// Pixel-coordinates.
// <../doc/brush-edge.png>
std::vector<IntLineSegment> brush_edge(const Brush&);

// Initializes the passed in AlphaMap from the given Brush
void init_brush_overlay(AlphaMap&, const Brush&);

// Return the top-left-most pixel intersected by the
// bounding-rectangle of the brush for this mouse-position.
UpperLeft brush_top_left(const Point& mousePos, const Brush&);

} // namespace
