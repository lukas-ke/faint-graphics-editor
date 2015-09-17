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

#ifndef FAINT_PY_COMMON_HH
#define FAINT_PY_COMMON_HH
#include "bitmap/color-counting.hh"
#include "bitmap/filter.hh"
#include "bitmap/gaussian-blur.hh"
#include "commands/blit-bitmap-cmd.hh"
#include "commands/draw-object-cmd.hh"
#include "commands/flip-rotate-cmd.hh"
#include "commands/function-cmd.hh"
#include "commands/rescale-cmd.hh"
#include "commands/resize-cmd.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "objects/objline.hh"
#include "python/py-function-error.hh"
#include "python/bound.hh" // for bare(T)
#include "text/formatting.hh"
#include "util-wx/clipboard.hh"
#include "util/default-settings.hh"

namespace faint{

// Python methods (as C++-templates) common to py-canvas, py-frame and
// py-bitmap.

using arg_type_t = decltype(METH_VARARGS);

template<typename T>
AppContext& common_get_app(const T& target){
  return target.ctx.app;
}

template<typename T>
auto common_get_tool_settings(const T& target){
  return common_get_app(target).GetToolSettings();
}

template<typename T>
auto common_get_bg(const T& target){
  return common_get_tool_settings(target).Get(ts_Bg);
}

template<typename T>
auto common_get_fg(const T& target){
  return common_get_tool_settings(target).Get(ts_Fg);
}

/* method: "aa_line((x0,y0,x1,y1),(r,g,b[,a]))\n
Experimental!\n
Draw an anti-aliased line from x0,y0 to x1,y1 with the specified color" */
template<typename T>
void Common_aa_line(T target, const IntLineSegment& line, const ColRGB& color){
  py_common_run_command(target,
    target_full_image(get_aa_line_command(line, color)));
}

/* method: "auto_crop()->p\n
Auto-crops the image. Returns true if the image was modified" */
template<typename T>
bool Common_auto_crop(T target){
  Command* cmd = get_auto_crop_command(bare(target).GetImage());
  if (cmd != nullptr){
    py_common_run_command(target, cmd);
    return true;
  }
  return false;
}

/* method: "blit(dst, (x,y), src_bmp)\n
Blits the src_bmp Bitmap onto self at x,y." */
template<typename T>
void Common_blit(T target, const IntPoint& pos, const Bitmap& bmp){
  py_common_run_command(target, get_blit_bitmap_command(pos, bmp));
}

/* method: "boundary_fill((x,y), fill, boundary_color)\n
Boundary fill from x,y with fill, up to the boundary_color." */
template<typename T>
void Common_boundary_fill(T target, const IntPoint& pos, const Paint& fill,
  const Color& boundary)
{
  if (!fully_positive(pos)){
    throw ValueError("Fill origin position must be positive");
  }
  if (!contains_pos(target, pos)){
    throw ValueError("Fill origin position outside image");
  }
  py_common_run_command(target,
    target_full_image(get_boundary_fill_command(pos, fill, boundary)));
}

/* method: "clear(paint)\n
Clear the image with the specified color, pattern or gradient" */
template<typename T>
void Common_clear(T target, const Paint& paint){
  py_common_run_command(target,
    target_full_image(get_clear_command(paint)));
}

/* method: "color_count() -> count\n
Returns the number of distinct colors in the image" */
template<typename T>
int Common_color_count(T target){
  return bare(target).GetBackground().Visit(
    [](const Bitmap& bmp){
      return count_colors(bmp);
    },
    [](const ColorSpan&){
      return 1;
    });
}

/* method: "desaturate()\n
Desaturate the image." */
template<typename T>
void Common_desaturate(T target){
  py_common_run_command(target,
    target_full_image(get_desaturate_simple_command()));
}

/* method: "desaturate_weighted()\n
Desaturate the image with weighted intensity." */
template<typename T>
void Common_desaturate_weighted(T target){
  py_common_run_command(target,
    target_full_image(get_desaturate_weighted_command()));
}

/* method: "flip_horizontally()\n
Flip the image horizontally (across the vertical axis)." */
template<typename T>
void Common_flip_horizontally(T target){
  py_common_run_command(target,
    target_full_image(function_command("Flip horizontally",
      [=](Bitmap& bmp){bmp = flip(bmp, along(Axis::HORIZONTAL));})));
}

/* method: "flip_vertically()\n
Flip the image vertically (across the horizontal axis)." */
template<typename T>
void Common_flip_vertically(T target){
  py_common_run_command(target,
    target_full_image(function_command("Flip vertically",
      [=](Bitmap& bmp){bmp = flip(bmp, along(Axis::VERTICAL));})));
}

/* method: "fill((x,y),paint)\n
Flood fill at x,y with paint." */
template<typename T>
void Common_fill(T target, const IntPoint& pos, const Paint& fill){
  if (!fully_positive(pos)){
    throw ValueError("Fill origin position must be positive");
  }
  if (!contains_pos(target, pos)){
    throw ValueError("Fill origin position outside image");
  }
  py_common_run_command(target,
    target_full_image(get_flood_fill_command(pos, fill)));
}

/* method: "gaussian_blur(sigma)\n
Blurs the image with a gaussian kernel." */
template<typename T>
void Common_gaussian_blur(T target, coord sigma){
  if (sigma <= 0){
    throw ValueError("Sigma must be > 0");
  }

  py_common_run_command(target,
    target_full_image(function_command("Gaussian blur",
      [=](Bitmap& bmp){
          bmp = gaussian_blur_exact(bmp, sigma);
        })));
}

/* method: "invert()\n
Invert the colors of the image" */
template<typename T>
void Common_invert(T target){
  py_common_run_command(target,
    target_full_image(get_invert_command()));
}

/* method: "replace_color(old, new)\n
Replaces all pixels matching the color old with the color new.\n
The colors are specified as r,g,b[,a]-tuples" */
template<typename T>
void Common_replace_color(T target, const Color& oldColor,
  const Paint& replacement)
{
  py_common_run_command(target,
    target_full_image(get_replace_color_command(Old(oldColor),
      replacement)));
}

/* method: "rotate(a[, bg])\n
Rotate the image a-radians, using the specified background color." */
template<typename T>
void Common_rotate(T target, const Angle& angle, const Optional<Paint>& bg){
  py_common_run_command(target,
    rotate_image_command(angle,
      bg.Or(common_get_bg(target))));
}

/* method: "sepia(intensity)\n
Applies a horrendous sepia filter on the image." */
template<typename T>
void Common_sepia(T target, int intensity){
  py_common_run_command(target,
    target_full_image(get_sepia_command(intensity)));
}

/* method: "Common_set_threshold(low, high[, c1, c2])\n
Assigns pixels with lightness:\n:
 - between low<->high to c1,\n
 - outside low<->high to c2.\n\n
The active foreground and background is used if c1, c2 omitted.\n\n
The valid range for low and high is 0.0->1.0, where 1.0 corresponds\n
to R=255, G=255 and B=255." */
template<typename T>
void Common_set_threshold(T target, const std::pair<double, double>& range,
  const Optional<Paint>& paintIn, const Optional<Paint>& paintOut)
{

  const auto lower = constrained(Min(0.0), range.first, Max(1.0));
  const auto upper = constrained(Min(0.0), range.second, Max(1.0));
  const auto r = fractional_bounded_interval<threshold_range_t>(lower, upper);
  py_common_run_command(target,
    target_full_image(get_threshold_command(r,
      paintIn.Or(common_get_fg(target)),
      paintOut.Or(common_get_bg(target)))));
}

template<typename T>
void Common_apply_paste(T target, const IntPoint& pos, const Bitmap& bmp){
  py_common_run_command(target,
    get_insert_raster_bitmap_command(bmp, pos,
      bare(target).GetRasterSelection(),
      common_get_tool_settings(target),
      "Paste Bitmap"));
}

/* method: "paste(x,y)\n
Pastes a bitmap from the clipboard to (x,y)" */
template<typename T>
void Common_paste(T target, const IntPoint& pos){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw ValueError("Failed opening clipboard");
  }
  clipboard.GetBitmap().Visit(
    [&target, &pos](const Bitmap& bmp){
      Common_apply_paste<T>(target, pos, bmp);
    },
    [](){
      throw ValueError("No bitmap in clipboard");
    });
}

/* method: "quantize()\n
Reduce the colors in the image to at most 256." */
template<typename T>
void Common_quantize(T target){
  py_common_run_command(target,
    target_full_image(get_quantize_command()));
}

/* method: "pixelize(width)\n
Pixelize the image with the given width" */
template<typename T>
void Common_pixelize(T target, const pixelize_range_t& width){
  py_common_run_command(target,
    target_full_image(get_pixelize_command(width)));
}

/* method: "erase_but_color(keptColor[,eraseColor])\n
Replaces all colors, except keptColor, with eraseColor. Uses the
current secondary color if keptColor is omitted." */
template<typename T>
void Common_erase_but_color(T target, const Color& keep,
  const Optional<Paint>& eraser)
{
  if (!eraser.IsSet()){
    Paint bg = common_get_bg(target);
    if (bg == keep){
      // Return without error when retrieved bg is same
      return;
    }
    py_common_run_command(target,
      target_full_image(get_erase_but_color_command(keep, bg)));
  }
  else{
    if (keep == eraser.Get()){
      // Consider explicitly replacing everyting but color with color
      // an error
      throw ValueError("Same erase color as the kept color");
    }
    py_common_run_command(target,
      target_full_image(get_erase_but_color_command(keep, eraser.Get())));
  }
}

/* method: "replace_alpha(r,g,b)\n
Blends alpha towards the specified color" */
template<typename T>
void Common_replace_alpha(T target, const ColRGB& color){
  py_common_run_command(target,
    target_full_image(get_blend_alpha_command(color)));
}

using color_value_t = StaticBoundedInt<0,255>;

/* method: "set_alpha(a)\n
Sets the alpha component of all pixels to a" */
template<typename T>
void Common_set_alpha(T target, const color_value_t& alpha){
  py_common_run_command(target,
    target_full_image(get_set_alpha_command(static_cast<uchar>(
      alpha.GetValue()))));
}

/* method: "color_balance((r0,r1),(g0,g1),(b0,b1))\n
Stretches the specified color intervals to [0,255]" */
template<typename T>
void Common_color_balance(T target, const color_range_t& r,
  const color_range_t& g,
  const color_range_t& b)
{
  py_common_run_command(target,
    target_full_image(function_command("Color balance",
      color_balance, r, g, b)));
}

} // namespace

#endif
