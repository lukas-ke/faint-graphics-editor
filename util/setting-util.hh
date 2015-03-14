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

#ifndef FAINT_SETTING_UTIL_HH
#define FAINT_SETTING_UTIL_HH
#include "util/distinct.hh"
#include "util/setting-id.hh"

namespace faint{

class Brush;
class Padding;
class RasterSelection;

bool alpha_blending(const Settings&);

bool anti_aliasing(const Settings&);

Settings bitmap_mask_settings(bool maskEnabled, const Paint& bg, bool alpha);

bool border(const Settings&);

bool dashed(const Settings&);

Settings eraser_rectangle_settings(const Paint& eraser);

// True if the fillstyle indicates filled inside
bool filled(FillStyle);

// True if the settings has a ts_FillStyle and it indicates filled
// inside.
bool filled(const Settings&);

// Tools can temporarily swap the foreground and background-colors
// (typically by drawing with right mouse button).
// For this, ts_SwapColors is set, rather than actually swapping
// ts_Bg and ts_Fg.  For objects (and some commands), the
// distinction is meaningless, so this function swaps ts_Fg and
// ts_Bg and clears ts_SwapColors.
void finalize_swap_colors(Settings&);

// Same as finalize_swap_colors, but erases the background and the
// swap colors flag.
void finalize_swap_colors_erase_bg(Settings&);

// Returns the background, taking ts_SwapColors in account
Paint get_bg(const Settings&);

Brush get_brush(const Settings&);

// Returns the foreground, taking ts_SwapColors in account
Paint get_fg(const Settings&);

class Filter;
Filter* get_filter(const Settings& s);

// Get extra size required for filters and Line width
Padding get_padding(const Settings&);
Settings get_selection_settings(const RasterSelection&);

bool is_object(Layer);
bool is_raster(Layer);

// Creates hit-test mask settings for fillable objects (ellipses,
// polygons etc.), from the actual objSettings
Settings mask_settings_fill(const Settings& objSettings);

// Creates hit test mask settings for unclosed paths (non-fillable objects)
Settings mask_settings_line(const Settings&);

// True if the background is masked (ts_BackgroundStyle)
bool masked_background(const Settings&);

// Returns settings without the background color (ts_Bg), and
// removes any swapping (ts_SwapColors)
Settings remove_background_color(const Settings&);

// Returns whether the ts_Fg or the ts_Bg is used to fill
// with the given fill style.
ColorSetting setting_used_for_fill(FillStyle);
ColorSetting the_other_one(const ColorSetting&);

// Returns a copy of those settings updated with those.
// Does not add new settings to these, only updates existing.
Settings updated(const Settings& these, const Settings& those);

// Adds filling to the passed in style, without modifying the border
// style
FillStyle with_fill(FillStyle);

class category_settingutil;
using start_enabled = Distinct<bool, category_settingutil, 0>;

// Add a a ts_EditPoints setting, possibly enabling it. This can be
// used to add point editing to an object, which should have
// point-editing, created by a tool which should not.
Settings with_point_editing(const Settings&, const start_enabled&);

} // namespace

#endif
