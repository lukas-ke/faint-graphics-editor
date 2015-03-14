// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_PY_LESS_COMMON_HH
#define FAINT_PY_LESS_COMMON_HH
#include "geo/points.hh"

namespace faint{

// Python methods for py-canvas and py-frame, but not py-bitmap.

/* method: "insert_bitmap(x,y, bmp)\n
Inserts bmp floating at x,y" */
template<typename T>
void Common_insert_bitmap(T target, const IntPoint& pos, const Bitmap& bmp){
  python_run_command(target,
    get_insert_raster_bitmap_command(bmp, pos,
      target.GetRasterSelection(),
      get_app_context().GetToolSettings(),
      "Insert Bitmap"));
}

/* method: "pinch_whirl(pinch, whirl)\n
Pinch (-1.0 to 1.0) and whirl (-2pi to 2pi) the image." */
template<typename T>
void Common_pinch_whirl(T target, coord pinch, const Angle& whirl){
  python_run_command(target,
    target_full_image(get_pinch_whirl_command(pinch, whirl)));
}

/* method: "scale_bilinear(w,h)\n
Scale the image to the specified size with bilinear interpolation" */
template<typename T>
void Common_scale_bilinear(T target, const IntSize& size){
  python_run_command(target, rescale_command(size, ScaleQuality::BILINEAR));
}

/* method: "scale_nearest(w,h)\n
Scale the image to the specified size with nearest neighbour
interpolation" */
template<typename T>
void Common_scale_nearest(T target, const IntSize& size){
  python_run_command(target, rescale_command(size, ScaleQuality::NEAREST));
}

/* method: "set_rect((x,y,w,h)[,bg])\n
Sets the image size to w,h extending from x,y.\nx and y may be negative" */
template<typename T>
void Common_set_rect(T target, const IntRect& rect, const Optional<Paint>& bg){
  if (empty(rect)){
    throw ValueError("Empty rectangle specified.");
  }
  python_run_command(target,
    resize_command(rect,
      bg.Or(get_app_context().GetToolSettings().Get(ts_Bg))));
}

template<typename T>
void Common_draw_object(T& target, its_yours_t<Object> object){
  python_run_command(target, draw_object_command(object));
}

/* method: "line(x0,y0,x1,y1)\n
Draw a line from x0,y0 to x1,y1" */
template<typename T>
void Common_line(T target, const LineSegment& line){
  Settings s(default_line_settings());
  s.Update(get_app_context().GetToolSettings());
  s.Set(ts_AntiAlias, false);
  Common_draw_object(target, its_yours(
   create_line_object(Points({PathPt::MoveTo(line.p0),
     PathPt::LineTo(line.p1)}), s)));
}

} // namespace

#endif
