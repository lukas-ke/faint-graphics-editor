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

#include "bitmap/alpha-map.hh"
#include "bitmap/brush.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/size.hh"
#include "render-brush.hh"

namespace faint{

std::vector<IntLineSegment> brush_edge(const Brush& b){
  IntSize sz(b.GetSize());
  std::vector<IntLineSegment> lines;
  for (int y = -1; y <= sz.h; y++){
    bool xTop = false;
    int xTopStart = 0;
    bool xBottom = false;
    int xBottomStart = 0;

    for (int x = 0; x <= sz.w; x++){
      // Pixel top side
      bool scanOut = (x != sz.w && y < sz.h - 1) &&
        (y == -1 || b.Get(x,y) == 0);
      if (scanOut){
        if (b.Get(x,y + 1) != 0){
          if (!xTop){
            xTopStart = x;
            xTop = true;
          }
        }
        else if (xTop){
          xTop = false;
          lines.push_back(IntLineSegment({xTopStart, y + 1},
                {x, y + 1}));
        }
      }
      else if (xTop){
        xTop = false;
        lines.push_back(IntLineSegment({xTopStart, y + 1},
              {x, y + 1}));
      }

      // Pixel bottom side
      scanOut = (x != sz.w && y > 0) &&
        (y == sz.h || b.Get(x,y) == 0);
      if (scanOut){
        if (b.Get(x, y - 1) != 0){
          if (!xBottom){
            xBottomStart = x;
            xBottom = true;
          }
        }
        else if (xBottom){
          xBottom = false;
          lines.push_back(IntLineSegment({xBottomStart,y}, {x, y}));
        }

      }
      else {
        if (xBottom){
          xBottom = false;
          lines.push_back(IntLineSegment({xBottomStart, y}, {x, y}));
        }
      }
    }
  }

  // Left/Right
  for (int x = -1; x <= sz.w; x++){
    bool yLeft = false;
    int yLeftStart = 0;

    bool yRight = false;
    int yRightStart = 0;

    for (int y = 0; y <= sz.h; y++){
      // Pixel left side
      bool scanOut = (x < sz.w - 1 && y != sz.h) &&
        (x == -1 || b.Get(x,y) == 0);
      if (scanOut){
        if (b.Get(x+1,y) != 0){
          if (!yLeft){
            yLeftStart = y;
            yLeft = true;
          }
        }
        else if (yLeft){
          yLeft = false;
          lines.push_back(IntLineSegment({x + 1, yLeftStart},
            {x + 1, y}));
        }
      }
      else {
        if (yLeft){
          yLeft = false;
          lines.push_back(IntLineSegment({x + 1, yLeftStart},
            {x + 1, y}));
        }
      }

      // Pixel right side
      scanOut = (x > 0 && y != sz.h) &&
        (x == sz.w || b.Get(x,y) == 0);
      if (scanOut){
        if (b.Get(x - 1,y) != 0){
          if (!yRight){
            yRightStart = y;
            yRight = true;
          }
        }
        else if (yRight){
          yRight = false;
          lines.push_back(IntLineSegment({x,yRightStart}, {x, y}));
        }

      }
      else {
        if (yRight){
          yRight = false;
          lines.push_back(IntLineSegment({x,yRightStart}, {x, y}));
        }
      }
    }
  }
  return lines;
}

void init_brush_overlay(AlphaMap& m, const Brush& b){
  IntSize brushSz = b.GetSize();
  m.Reset(brushSz);
  for (int y = 0; y != brushSz.h; y++){
    for (int x = 0; x != brushSz.w; x++){
      m.Add(x, y, b.Get(x,y));
    }
  }
}

UpperLeft brush_top_left(const Point& mousePos, const Brush& brush){
  IntSize size(brush.GetSize());
  return (area(size) == 1) ?
    UpperLeft(truncated(mousePos)) :
    UpperLeft(rounded(mousePos - point_from_size(floated(brush.GetSize())) / 2));
}

} // namespace
