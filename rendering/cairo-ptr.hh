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

#ifndef FAINT_CAIRO_PTR_HH
#define FAINT_CAIRO_PTR_HH
#include <memory>
#include "cairo.h"
#include "pango/pangocairo.h"

namespace faint{

// unique_ptr definitions for Pango and Cairo types

template<typename T, void(*func)(T*)>
struct Destroyer{
  void operator()(T* item){
    func(item);
  }
};

template<typename T>
struct UnRef{
  void operator()(T* item){
    g_object_unref(item);
  }
};

struct NoOp{
  template<typename T>
  void operator()(T*){}
};

#define FAINT_DELETER(T, DEL)std::unique_ptr<T, Destroyer<T, DEL>>
template<typename T>
using unref_ptr_t = std::unique_ptr<T, UnRef<T>>;

using surface_ptr_t = FAINT_DELETER(cairo_surface_t, cairo_surface_destroy);
using cairo_ptr_t = FAINT_DELETER(cairo_t, cairo_destroy);
using pattern_ptr_t = FAINT_DELETER(cairo_pattern_t, cairo_pattern_destroy);
using font_options_ptr_t = FAINT_DELETER(cairo_font_options_t,
  cairo_font_options_destroy);
using font_metrics_ptr_t = FAINT_DELETER(PangoFontMetrics,
  pango_font_metrics_unref);

using path_ptr_t = FAINT_DELETER(cairo_path_t, cairo_path_destroy);

using layout_ptr_t = unref_ptr_t<PangoLayout>;
using font_ptr_t = unref_ptr_t<PangoFont>;
using pango_context_ptr_t = unref_ptr_t<PangoContext>;
using font_description_ptr_t = FAINT_DELETER(PangoFontDescription,
  pango_font_description_free);
using font_map_ptr_t = std::unique_ptr<PangoFontMap, NoOp>;

path_ptr_t manage(cairo_path_t* raw){
  return path_ptr_t(raw, path_ptr_t::deleter_type());
}

surface_ptr_t manage(cairo_surface_t* raw){
  return surface_ptr_t(raw, surface_ptr_t::deleter_type());
}

font_map_ptr_t manage(PangoFontMap* raw){
  return font_map_ptr_t(raw, font_map_ptr_t::deleter_type());
}

pango_context_ptr_t manage(PangoContext* raw){
  return pango_context_ptr_t(raw, pango_context_ptr_t::deleter_type());
}

layout_ptr_t manage(PangoLayout* raw){
  return layout_ptr_t(raw, layout_ptr_t::deleter_type());
}

font_ptr_t manage(PangoFont* raw){
  return font_ptr_t(raw, font_ptr_t::deleter_type());
}

font_metrics_ptr_t manage(PangoFontMetrics* raw){
  return font_metrics_ptr_t(raw, font_metrics_ptr_t::deleter_type());
}

font_options_ptr_t manage(cairo_font_options_t* raw){
  return font_options_ptr_t(raw, font_options_ptr_t::deleter_type());
}

font_description_ptr_t manage(PangoFontDescription* raw){
  return font_description_ptr_t(raw, font_description_ptr_t::deleter_type());
}

pattern_ptr_t manage(cairo_pattern_t* raw){
  return pattern_ptr_t(raw, pattern_ptr_t::deleter_type());
}

cairo_ptr_t cairo_create(surface_ptr_t& surface){
  return cairo_ptr_t(cairo_create(surface.get()),
    cairo_ptr_t::deleter_type());
}

// Macros for forwarding calls to cairo/pango functions with
// unpacking of the unique_ptr, to reduce the amount of .get()-calls
// required in cairo-context.
#define CAIRO_FWD(NAME)\
template<typename T, typename ...Args>\
  inline void NAME(T& t, Args&&...args){ \
  return NAME(t.get(), std::forward<Args>(args)...); \
}

#define CAIRO_FWD_2(NAME)\
  template<typename T1, typename T2, typename ...Args> \
  inline void NAME(T1& v1, T2& v2, Args&&...args){ \
  return NAME(v1.get(), v2.get(), std::forward<Args>(args)...);  \
}

CAIRO_FWD(cairo_arc)
CAIRO_FWD(cairo_arc_negative)
CAIRO_FWD(cairo_clip)
CAIRO_FWD(cairo_close_path)
CAIRO_FWD(cairo_curve_to)
CAIRO_FWD(cairo_fill)
CAIRO_FWD(cairo_fill_preserve)
CAIRO_FWD(cairo_line_to)
CAIRO_FWD(cairo_move_to)
CAIRO_FWD(cairo_new_sub_path)
CAIRO_FWD(cairo_paint)
CAIRO_FWD(cairo_pattern_add_color_stop_rgba)
CAIRO_FWD(cairo_pattern_set_matrix)
CAIRO_FWD(cairo_rectangle)
CAIRO_FWD(cairo_restore)
CAIRO_FWD(cairo_rotate)
CAIRO_FWD(cairo_save)
CAIRO_FWD(cairo_scale)
CAIRO_FWD(cairo_set_operator)
CAIRO_FWD(cairo_set_source_rgba)
CAIRO_FWD(cairo_stroke)
CAIRO_FWD(cairo_translate)
CAIRO_FWD_2(cairo_set_source)
CAIRO_FWD_2(cairo_set_source_rgba)
CAIRO_FWD_2(cairo_set_source_surface)
CAIRO_FWD_2(pango_layout_set_font_description)

} // namespace

#endif
