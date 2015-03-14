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

#ifndef FAINT_ITER_BMP_HH
#define FAINT_ITER_BMP_HH

// Macro for iterating over a bitmap in row-major order. Used like
// this:
//
//   for (ITER_XY(x,y,bmp)){
//     get_color(bmp, {x,y}); // (For example)
//   }
//
#define ITER_XY(X, Y, BMP) int Y = 0; Y != BMP.m_h; Y++)for (int X = 0; X != BMP.m_w; X++

// Similar to ITER_XY, but also provides the pixel data offset for X, Y.
#define ITER_XY_OFFSET(X, Y, OFFSET, BMP) int Y = 0; Y != BMP.m_h; Y++)for (int X = 0, OFFSET=Y*BMP.GetStride(); X != BMP.m_w; X++, OFFSET+=4

#endif
