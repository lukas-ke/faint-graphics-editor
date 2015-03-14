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

#include <cassert>
#include "bitmap/brush.hh"
#include "bitmap/filter.hh"
#include "objects/object.hh"
#include "rendering/filter-class.hh"
#include "util/default-settings.hh"
#include "util/raster-selection.hh"
#include "util/setting-util.hh"

namespace faint{

// Adds filling to the ts_FillStyle settings without affecting the
// border. Note: Asserts that ts_FillStyle exists.
static void add_fill(Settings& s){
  assert(s.Has(ts_FillStyle));
  FillStyle fillStyle = s.Get(ts_FillStyle);
  if (fillStyle == FillStyle::BORDER){
    s.Set(ts_FillStyle, FillStyle::BORDER_AND_FILL);
  }
}

bool alpha_blending(const Settings& s){
  return s.GetDefault(ts_AlphaBlending, true);
}

bool anti_aliasing(const Settings& s){
  return s.GetDefault(ts_AntiAlias, false);
}

static bool can_use_as_mask(const Paint& paint){
  return paint.IsColor();
}

Settings bitmap_mask_settings(bool maskEnabled, const Paint& bg, bool alpha){
  Settings s(default_bitmap_settings());
  s.Set(ts_Bg, Paint(bg));
  s.Set(ts_BackgroundStyle,
    can_use_as_mask(bg) && maskEnabled  ?
    BackgroundStyle::MASKED : BackgroundStyle::SOLID);
  s.Set(ts_AlphaBlending, alpha);
  return s;
}

bool border(const Settings& s){
  if (!s.Has(ts_FillStyle)){
    return false;
  }
  FillStyle f = s.Get(ts_FillStyle);
  return f == FillStyle::BORDER_AND_FILL || f == FillStyle::BORDER;
}

bool dashed(const Settings& s){
  return s.Has(ts_LineStyle) && s.Get(ts_LineStyle) == LineStyle::LONG_DASH;
}

Settings eraser_rectangle_settings(const Paint& eraser){
  Settings s(default_rectangle_settings());
  s.Set(ts_FillStyle, FillStyle::FILL);
  s.Set(ts_AntiAlias, false);
  s.Set(ts_Fg, eraser);
  s.Set(ts_Bg, eraser);
  return s;
}

bool filled(FillStyle fillStyle){
  return fillStyle == FillStyle::
    FILL || fillStyle == FillStyle::BORDER_AND_FILL;
}

bool filled(const Settings& s){
  return filled(s.GetDefault(ts_FillStyle, FillStyle::BORDER));
}

void finalize_swap_colors(Settings& s){
  if (s.Has(ts_SwapColors) && s.Get(ts_SwapColors)){
    Paint fg = s.Get(ts_Bg);
    s.Set(ts_Bg, s.Get(ts_Fg));
    s.Set(ts_Fg, fg);
    s.Set(ts_SwapColors, false);
  }
}

void finalize_swap_colors_erase_bg(Settings& s){
  if (s.Has(ts_SwapColors)){
    if (s.Get(ts_SwapColors)){
      Paint fg = s.Get(ts_Bg);
      s.Set(ts_Bg, s.Get(ts_Fg));
      s.Set(ts_Fg, fg);
    }
    s.Erase(ts_SwapColors);
  }
  s.Erase(ts_Bg);
}

Paint get_bg(const Settings& s){
  bool explicitSwap = s.GetDefault(ts_SwapColors, false);
  bool fillOnlySwap = s.Has(ts_FillStyle) &&
    s.Get(ts_FillStyle) == FillStyle::FILL;
  bool swap = explicitSwap ^ fillOnlySwap;
  return swap ? s.Get(ts_Fg) : s.Get(ts_Bg);
}

Brush get_brush(const Settings& s){
  BrushShape shape = s.Get(ts_BrushShape);
  int size = s.Get(ts_BrushSize);
  if (shape == BrushShape::SQUARE){
    return rect_brush(size);
  }
  else if (shape == BrushShape::CIRCLE){
    return circle_brush(size);
  }
  else if (shape == BrushShape::EXPERIMENTAL){
    return experimental_brush(size);
  }
  else {
    assert(false);
    return Brush(IntSize::Both(size));
  }
}

Paint get_fg(const Settings& s){
  bool explicitSwap = s.GetDefault(ts_SwapColors, false);
  return explicitSwap ? s.Get(ts_Bg) : s.Get(ts_Fg);
}

Filter* get_filter(const Settings& s){
  int filterNum = s.GetDefault(ts_Filter, 0);
  if (filterNum == 0){
    return nullptr;
  }
  else if (filterNum == 1){
    return get_stroke_filter();
  }
  else if (filterNum == 2){
    return get_pixelize_filter();
  }
  else if (filterNum == 3){
    return get_pinch_whirl_filter();
  }
  else if (filterNum == 4){
    return get_invert_filter();
  }
  else if (filterNum == 5){
    return get_shadow_filter();
  }
  else {
    assert(false);
    return nullptr;
  }
}

Padding get_padding(const Settings& s){
  Padding p = Padding::All(border(s) ? rounded(s.Get(ts_LineWidth)) : 0) +
    Padding::Divide(s.GetDefault(ts_BrushSize, 0));
  Filter* f = get_filter(s);
  if (f == nullptr){
    return p;
  }
  return p + f->GetPadding();
}

// Todo: Move to selection util or smth
Settings get_selection_settings(const RasterSelection& selection){
  SelectionOptions options = selection.GetOptions();
  Settings s;
  s.Set(ts_Bg, options.bg);
  s.Set(ts_BackgroundStyle,
    options.mask ? BackgroundStyle::MASKED : BackgroundStyle::SOLID);
  s.Set(ts_AlphaBlending, options.alpha);
  return s;
}

bool is_object(Layer layer){
  return layer == Layer::OBJECT;
}

bool is_raster(Layer layer){
  return layer == Layer::RASTER;
}

Settings mask_settings_fill(const Settings& objSettings){
  Settings s(objSettings);
  s.Set(ts_Fg, Paint(mask_edge));
  bool hasBorder = border(s);
  bool hasFill = filled(s);

  if (hasBorder && hasFill){
    s.Set(ts_Bg, Paint(mask_fill));
  }
  else if (hasBorder){
    // The transparent inside of the object should be filled with
    // the inside-indicating color in the mask
    s.Set(ts_Bg, Paint(mask_no_fill));
    add_fill(s);
  }
  else if (hasFill){
    s.Set(ts_Fg, Paint(mask_fill));
  }
  else {
    // Neither fill nor stroke (possible e.g. when loaded from SVG)
    s.Set(ts_Bg, Paint(mask_no_fill));
    s.Set(ts_Fg, Paint(mask_no_fill));
    add_fill(s);
  }
  return s;
}

Settings mask_settings_line(const Settings& objSettings){
  Settings s(objSettings);
  s.Set(ts_Fg, Paint(mask_edge));
  return s;
}

bool masked_background(const Settings& s){
  return s.Get(ts_BackgroundStyle) == BackgroundStyle::MASKED;
}

Settings remove_background_color(const Settings& s){
  Settings out(s);
  if (out.Has(ts_Bg)){
    out.Erase(ts_Bg);
  }
  if (out.GetDefault(ts_SwapColors, false)){
    out.Set(ts_Fg, s.Get(ts_Bg));
    out.Erase(ts_SwapColors);
  }
  return out;
}

ColorSetting setting_used_for_fill(FillStyle fillStyle){
  return fillStyle == FillStyle::FILL ?
    ts_Fg : ts_Bg;
}

ColorSetting the_other_one(const ColorSetting& s){
  return s == ts_Fg ? ts_Bg : ts_Fg;
}

Settings updated(const Settings& these, const Settings& those){
  Settings s(these);
  s.Update(those);
  return s;
}

FillStyle with_fill(FillStyle fillStyle){
  if (fillStyle == FillStyle::BORDER){
    return FillStyle::BORDER_AND_FILL;
  }
  return fillStyle;
}

Settings with_point_editing(const Settings& s,
  const start_enabled& startEnabled)
{
  if (s.Has(ts_EditPoints)){
    return s;
  }
  Settings s2(s);
  s2.Set(ts_EditPoints, startEnabled.Get());
  return s2;
}

} // namespace
