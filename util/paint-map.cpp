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

#include <algorithm> // For std::unique
#include <numeric> // For std::accumulate
#include <set>
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/paint.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/point-range.hh"
#include "util/color-bitmap-util.hh"
#include "util/paint-map.hh"

namespace faint{

using paint_vec_t = std::vector<Paint>;
using color_map_t = std::vector<paint_vec_t>;

static int get_max_row(const color_map_t& colors){
  assert(!colors.empty());
  return resigned(colors.size()) - 1;
}

static int get_max_column(const color_map_t& colors){
  assert(!colors.empty());
  int maxCol = resigned(colors[0].size());
  for (size_t i = 1; i != colors.size(); i++){
    maxCol = std::max(resigned(colors[i].size()), maxCol);
  }
  return maxCol;
}

static paint_vec_t& get_shortest_row(color_map_t& colors){
  assert(!colors.empty());
  color_map_t::iterator it = std::min_element(begin(colors), end(colors),
    [](const paint_vec_t& lhs, const paint_vec_t& rhs){
      return lhs.size() < rhs.size();});
  return *it;
}

static IntPoint get_max_coords(const std::vector<IntPoint>& v){
  assert(!v.empty());
  return std::accumulate(begin(v), end(v), v.front(),
    [](const IntPoint& lhs, const IntPoint& rhs){
      return max_coords(lhs, rhs);});
}

static IntPoint get_min_coords(const std::vector<IntPoint>& v){
  assert(!v.empty());
  return std::accumulate(begin(v), end(v), v.front(),
    [](const IntPoint& lhs, const IntPoint& rhs){
      return min_coords(lhs, rhs);});
}

static bool has_index(const color_map_t& colors, const IntPoint& pt){
  if (pt.x < 0 || pt.y < 0 || colors.size() <= static_cast<size_t>(pt.y)){
    return false;
  }
  return to_size_t(pt.x) < colors[to_size_t(pt.y)].size();
}

static void erase_index(color_map_t& colors, const IntPoint& index){
  assert(has_index(colors, index));
  paint_vec_t& row = colors[to_size_t(index.y)];
  row.erase(begin(row) + index.x);
}

static const Paint& get_color(const color_map_t& colors, const IntPoint& pt){
  assert(pt.x >= 0);
  assert(pt.y >= 0);
  assert(colors.size() > to_size_t(pt.y));
  const paint_vec_t& row = colors[to_size_t(pt.y)];
  assert(row.size() > static_cast<size_t>(pt.x));
  return row[to_size_t(pt.x)];
}

static bool lacks(const std::vector<IntPoint>& v, const IntPoint& p){
  return std::find(begin(v), end(v), p) == end(v);
}

static bool contains(const std::vector<IntPoint>& v, const IntPoint& p){
  return !lacks(v, p);
}

static std::vector<IntPoint> flood_fill_search(const color_map_t& colors,
  const IntPoint& pos)
{
  assert(has_index(colors, pos));
  std::vector<IntPoint> consider = {pos};
  const Paint& paint = get_color(colors, pos);
  std::vector<IntPoint> filled;

  // Note: Not using a range loop, since "consider" changes during iteration
  for (size_t i = 0; i != consider.size(); i++){
    const IntPoint& currPt = consider[i];
    if (!has_index(colors, currPt) || get_color(colors, currPt) != paint){
      continue;
    }
    { // Extra scope for "w"
      IntPoint w = currPt;
      for (;;){
        filled.push_back(w);
        w -= delta_x(1);

        if (!has_index(colors, w) || get_color(colors, w) != paint ||
          contains(filled, w))
        {
          break;
        }
        const IntPoint above(w - delta_y(1));
        if (has_index(colors, above)){
          const Paint& tempPaint(get_color(colors, above));
          if (tempPaint == paint && lacks(filled, above)){
            consider.push_back(above);
          }
        }

        const IntPoint below(w + delta_y(1));
        if (has_index(colors, below)){
          const Paint& tempPaint(get_color(colors, below));
          if (tempPaint == paint && lacks(filled, below)){
            consider.push_back(below);
          }
        }
      }
    }

    { // Extra scope for "e"
      IntPoint e = currPt;
      for (;;){
        filled.push_back(e);
        const IntPoint above = e - delta_y(1);
        if (has_index(colors, above)){
          const Paint& tempPaint(get_color(colors, above));
          if (tempPaint == paint && lacks(filled, above)){
            consider.push_back(above);
          }
        }

        const IntPoint below(e + delta_y(1));
        if (has_index(colors, below)){
          const Paint& tempPaint(get_color(colors, below));
          if (tempPaint == paint && lacks(filled, below)){
            consider.push_back(below);
          }
        }
        e += delta_x(1);
        if (!has_index(colors, e) || get_color(colors, e) != paint){
          break;
        }
      }
    }
  }
  filled.erase(std::unique(begin(filled), end(filled)), end(filled));
  return filled;
}

static void copy_paint(color_map_t& colors, const IntPoint& oldPos,
  const IntPoint& newPos, bool eraseOld)
{
  assert(has_index(colors, oldPos));
  assert(fully_positive(newPos));
  if (oldPos == newPos){
    return;
  }
  const Paint& paint(get_color(colors, oldPos));
  assert(static_cast<size_t>(newPos.y) < colors.size());
  paint_vec_t& row = colors[to_size_t(newPos.y)];
  if (row.size() <= to_size_t(newPos.x)){
    row.push_back(paint);
    if (eraseOld){
      erase_index(colors, oldPos);
    }
  }
  else {
    bool moveLeft = oldPos.x >= newPos.x;
    bool sameRow = newPos.y == oldPos.y;
    bool moveRight = !moveLeft;
    int insertOffset = moveRight && sameRow && eraseOld ? 1 : 0;
    int removeOffset = moveLeft && sameRow ? 1 : 0;
    row.insert(begin(row) + newPos.x + insertOffset, paint);
    if (eraseOld){
      erase_index(colors, oldPos + delta_x(removeOffset));
    }
  }
}

static Bitmap palette_bitmap(const color_map_t& colors,
  const CellSize& size,
  const CellSpacing& spacing,
  const Color& bgColor)
{
  if (colors.empty()){
    return Bitmap(size, bgColor);
  }

  int maxRow = get_max_row(colors);
  IntPoint maxPos(get_max_column(colors),
    maxRow);
  IntPoint offset(point_from_size(size + spacing));
  IntSize bitmapSize((maxPos.x + 1) * (size.w + spacing.w),
    (maxPos.y + 1) * (size.h + spacing.h));
  Bitmap bmp(bitmapSize, bgColor);
  std::set<IntPoint> ignore;
  for (int rowNum = 0; rowNum <= maxRow; rowNum++){
    const paint_vec_t& row(colors[to_size_t(rowNum)]);
    const int colSize = resigned(row.size());
    for (int colNum = 0; colNum != colSize; colNum++){
      const IntPoint pos(colNum, rowNum);
      if (ignore.find(pos) != end(ignore)){
        continue;
      }
      assert(has_index(colors, pos));
      const Paint& paint(get_color(colors, pos));
      std::vector<IntPoint> identical = flood_fill_search(colors, pos);
      assert(!identical.empty());
      ignore.insert(begin(identical), end(identical));
      IntPoint maxPos(get_max_coords(identical));
      IntPoint minPos(get_min_coords(identical));
      IntSize deltaCells(size_from_point(maxPos - minPos));
      IntSize size2(size * (deltaCells + IntSize(1,1)) + deltaCells * spacing);

      Bitmap cellBitmap = paint_bitmap(paint, size2, size2);
      for (auto pt : point_range(minPos, maxPos)){
        if (lacks(identical, pt)){
          fill_rect_color(cellBitmap,
            IntRect(IntPoint((pt.x - minPos.x)  * (size.w + spacing.w) - spacing.w,
                (pt.y - minPos.y) * (size.h + spacing.h) - spacing.h),
              size + 2 * spacing),
            color_transparent_white);
        }
      }
      blend(offsat(cellBitmap, minPos * offset), onto(bmp));
    }
  }
  return bmp;
}

class PaintMapImpl{
public:
  color_map_t colors;
};

PaintMap::PaintMap(){
  m_impl = new PaintMapImpl;
  m_impl->colors.push_back(paint_vec_t());
  m_impl->colors.push_back(paint_vec_t());
}

PaintMap::PaintMap(const PaintMap& other){
  m_impl = new PaintMapImpl;
  m_impl->colors = other.m_impl->colors;
}

PaintMap::~PaintMap(){
  delete m_impl;
}

void PaintMap::Append(const Paint& paint){
  color_map_t& colors = m_impl->colors;
  assert(!colors.empty()); // Must contain at least one row (possibly empty)
  paint_vec_t& row = get_shortest_row(colors);
  row.push_back(paint);
  return;
}

void PaintMap::Copy(const OldPos& oldPos, const NewPos& newPos){
  return copy_paint(m_impl->colors, oldPos.Get(), newPos.Get(), false);
}

Bitmap PaintMap::CreateBitmap(const CellSize& size, const CellSpacing& spacing,
  const Color& bgColor) const
{
  return palette_bitmap(m_impl->colors, size, spacing, bgColor);
}

void PaintMap::Erase(const CellPos& removePos){
  erase_index(m_impl->colors, removePos);
}

const Paint& PaintMap::Get(const CellPos& pos) const{
  assert(has_index(m_impl->colors, pos));
  return get_color(m_impl->colors, pos);
}

IntSize PaintMap::GetSize() const{
  return IntSize(get_max_column(m_impl->colors),
    get_max_row(m_impl->colors));
}

bool PaintMap::Has(const CellPos& pos) const{
  return has_index(m_impl->colors, pos);
}

void PaintMap::Move(const OldPos& oldPos, const NewPos& newPos){
  copy_paint(m_impl->colors, oldPos.Get(), newPos.Get(), true);
}

void PaintMap::Replace(const CellPos& pos, const Paint& paint){
  assert(has_index(m_impl->colors,pos));
  m_impl->colors[to_size_t(pos.y)][to_size_t(pos.x)] = paint;
}

PaintMap& PaintMap::operator=(const PaintMap& other){
  if (this == &other){
    return *this;
  }
  delete m_impl;
  m_impl = new PaintMapImpl;
  m_impl->colors = other.m_impl->colors;
  return *this;
}

void add_cell_border(Bitmap& bmp, const CellPos& pos, const CellSize& cellSize,
  const CellSpacing& cellSpacing, const Color& c)
{
  IntPoint offset(point_from_size(cellSize + cellSpacing));
  IntPoint topLeft(pos * offset);
  draw_rect(bmp,
    {topLeft, cellSize},
    {c, 2, LineStyle::SOLID});
}

CellPos view_to_cell_pos(const IntPoint& viewPos, const CellSize& cellSize,
  const CellSpacing& cellSpacing)
{
  IntPoint offset(point_from_size(cellSize + cellSpacing));
  assert(offset.x > 0 && offset.y > 0);
  return CellPos(viewPos /
    point_from_size(cellSize + cellSpacing));
}

} // namespace faint
