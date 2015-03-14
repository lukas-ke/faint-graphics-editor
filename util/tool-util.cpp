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

#include "app/canvas.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "geo/adjust.hh"
#include "geo/angle.hh"
#include "geo/geo-func.hh"
#include "objects/objtext.hh"
#include "util/image.hh"
#include "util/pos-info.hh"
#include "util/setting-util.hh"
#include "util/tool-util.hh"

namespace faint{

ColorSetting fg_or_bg(const PosInfo& info){
  return info.modifiers.RightMouse() ?
    ts_Bg : ts_Fg;
}

static Paint get_hit_object_paint(Hit hit, Object* obj){
  const Settings& s(obj->GetSettings());
  if (hit == Hit::INSIDE && s.Has(ts_FillStyle)){
    return s.Get(setting_used_for_fill(s.Get(ts_FillStyle)));
  }
  return s.Get(ts_Fg);
}

static bool transparent_inside_hit(const PosInfo& info){
  return info.hitStatus == Hit::INSIDE && !filled(info.object->GetSettings());
}

Color get_color(const Either<Bitmap, ColorSpan>& bg, const IntPoint& pos){
  return bg.Visit(
    [&](const Bitmap& bmp){
      assert(point_in_bitmap(bmp, pos));
      return get_color(bmp, pos);
    },
    [](const ColorSpan& span){
      return span.color;
    });
}

Paint get_hovered_paint(const PosInside& info,
  const include_hidden_fill& includeHiddenFill,
  const include_floating_selection& includeFloatingSelection)
{
  if (includeFloatingSelection.Get() && info->inSelection){
    const RasterSelection& selection = info->canvas.GetRasterSelection();
    if (selection.Floating()){
      const Bitmap& selBmp = selection.GetBitmap();
      return Paint(get_color(selBmp, floored(info->pos) - selection.TopLeft()));
    }
  }
  if (is_object(info->layerType) && object_color_region_hit(info)){
    if (includeHiddenFill.Get() || !transparent_inside_hit(info)){
      return get_hit_object_paint(info->hitStatus, info->object);
    }
  }

  return Paint(get_color(info->canvas.GetImage().GetBackground(),
    floored(info->pos)));
}

// Returns the topmost text object in the ObjComposite or 0 if no text
// object is contained by the composite.
static ObjText* get_text_from(Object* group){
  const int numObjects = group->GetObjectCount();
  for (int i = 0; i != numObjects; i++){
    ObjText* objText = dynamic_cast<ObjText*>(group->GetObject(i));
    if (objText != nullptr){
      return objText;
    }
  }
  return nullptr;
}

ObjText* hovered_selected_text(const PosInfo& info, SearchMode mode){
  if (!info.objSelected || info.object == nullptr){
    return nullptr;
  }

  ObjText* objText = dynamic_cast<ObjText*>(info.object);
  if (objText != nullptr || mode == SearchMode::exact_object){
    return objText;
  }
  return get_text_from(info.object);
}

bool has_color_setting(Object* object){
  const Settings& s(object->GetSettings());
  return s.Has(ts_Fg) || s.Has(ts_Bg);
}

bool object_color_region_hit(const PosInfo& info){
  return info.object != 0 &&
    (info.hitStatus == Hit::BOUNDARY || info.hitStatus == Hit::INSIDE) &&
    has_color_setting(info.object);
}

bool outside_canvas(const PosInfo& info){
  return outside_canvas_by(info, 0);
}

bool outside_canvas_by(const PosInfo& info, int pixels){
  if (info.pos.x + pixels < 0 || info.pos.y + pixels < 0){
    return true;
  }
  IntSize sz = info.canvas.GetSize();
  return info.pos.x - pixels >= sz.w ||
    info.pos.y - pixels >= sz.h;
}

Paint offsat_if_object_aligned(const Paint& p, const IntPoint& offset){
  return visit(p,
    [&](const Color&) -> Paint{
      return p;
    },
    [&](const Pattern& pattern){
      return pattern.GetObjectAligned() ?
        Paint(offsat(pattern, offset)) :
        p;
    },
    [&](const Gradient&){
      // Fixme: Gradient should also be offset somehow.
      return p;
    });
}

Point constrain_to_square(const Point& p0, const Point& p1, bool subPixel){
  return subPixel?
    adjust_to(p0, p1, 90_deg, 45_deg) :
    adjust_to(floated(floored(p0)), floated(floored(p1)), 90_deg, 45_deg);
}

} // namespace
