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

#ifndef FAINT_PAINT_MAP_HH
#define FAINT_PAINT_MAP_HH
#include "geo/int-point.hh"
#include "util/distinct.hh"

namespace faint{

class Bitmap;
class Color;
class IntSize;
class Paint;
class PaintMapImpl;

class category_paint_map;
using CellSize = Subtype<IntSize, category_paint_map, 0>;
using CellSpacing = Subtype<IntSize, category_paint_map, 1>;
using CellPos = Subtype<IntPoint, category_paint_map, 2>;
using NewPos = Order<CellPos>::New;
using OldPos = Order<CellPos>::Old;

class PaintMap{
  // A 2D grid of Paints. Accessed via CellPos-indexes.
  // Can also create a "palette"-Bitmap representing its content.
public:
  PaintMap();
  PaintMap(const PaintMap&);
  ~PaintMap();

  // Append the Paint at the end of to the shortest row
  void Append(const Paint&);

  // Clones from OldPos to NewPos. Shifts the current Paint at NewPos
  // (does not replace).
  void Copy(const OldPos&, const NewPos&);

  // Returns a Bitmap representation of the contents
  Bitmap CreateBitmap(const CellSize&, const CellSpacing&,
    const Color&) const;

  // Erases the Paint at CellPos. Shifts to fill holes
  void Erase(const CellPos&);

  // Returns the Paint at CellPos (for which Has() must be true)
  const Paint& Get(const CellPos&) const;

  // Returns the number of rows and columns. Note that individual rows
  // can be shorter than the column count
  IntSize GetSize() const;
  bool Has(const CellPos&) const;

  // Moves the cell at OldPos to NewPos. Shifts the current Paint at
  // NewPos. Shifts cells to fill the hole at OldPos.
  void Move(const OldPos&, const NewPos&);

  // Replaces the Paint at CellPos (for which Has() must be true)
  // with the specified Paint.
  void Replace(const CellPos&, const Paint&);
  PaintMap& operator=(const PaintMap&);
private:
  PaintMapImpl* m_impl;
};

// Adds a border around a cell for a bitmap created with
// PaintMap::CreateBitmap
void add_cell_border(Bitmap& bmp,
  const CellPos&,
  const CellSize&,
  const CellSpacing&,
  const Color&);

// Converts a point in view-coordinates relative to the top-left of a
// bitmap created with PaintMap::CreateBitmap into positions in a
// PaintMap.
CellPos view_to_cell_pos(const IntPoint&,
  const CellSize&,
  const CellSpacing&);

} // namespace faint

#endif
