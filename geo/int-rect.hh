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

#ifndef FAINT_INT_RECT_HH
#define FAINT_INT_RECT_HH
#include "geo/geo-fwd.hh"

namespace faint{

class IntRect{
public:
  IntRect();
  IntRect(const IntPoint&, const IntPoint&);
  IntRect(const IntPoint&, const IntSize&);
  static IntRect EmptyRect();
  IntSize GetSize() const;
  int Left() const;
  int Right() const;
  int Top() const;
  int Bottom() const;
  IntPoint TopLeft() const;
  IntPoint TopRight() const;
  IntPoint BottomLeft() const;
  IntPoint BottomRight() const;
  IntPoint MidTop() const;
  IntPoint MidBottom() const;
  IntPoint MidLeft() const;
  IntPoint MidRight() const;
  bool Contains(const IntPoint&) const;
  void MoveTo(const IntPoint&);

  int x;
  int y;
  int w;
  int h;
};

bool operator==(const IntRect&, const IntRect&);
bool operator!=(const IntRect&, const IntRect&);
int area(const IntRect&);
bool empty(const IntRect&);

// Returns the rectangle grown around its center by 2 * dx-pixels
// horizontally and 2 * dy vertically
// <../doc/inflated.png>
IntRect inflated(const IntRect&, int dx, int dy);
IntRect inflated(const IntRect&, int);

IntRect deflated(const IntRect&, int);
IntRect intersection(const IntRect&, const IntRect&);

// Returns the largest rectangle, or the SECOND argument if equal.
IntRect largest(const IntRect&, const IntRect&);

// Returns the smallest rectangle, or the FIRST argument if equal.
IntRect smallest(const IntRect&, const IntRect&);

IntRect translated(const IntRect&, const IntPoint&);
IntRect union_of(const IntRect&, const IntRect&);

} // namespace

#endif
