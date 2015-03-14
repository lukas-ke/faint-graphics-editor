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

#include <algorithm>
#include <cassert>
#include <memory>
#include "objects/object.hh"
#include "rendering/faint-dc.hh"
#include "util/command-util.hh"
#include "util/image.hh"

namespace faint{

template<Command*(*func)(const objects_t&, const Image&)>
bool got(const Image& image, const objects_t& objects){
  return !objects.empty() &&
    std::unique_ptr<Command>(func(objects, image)) != nullptr;
}

bool can_move_backward(const Image& image, const objects_t& objects){
  return got<get_objects_backward_command>(image, objects);
}

bool can_move_forward(const Image& image, const objects_t& objects){
  return got<get_objects_forward_command>(image, objects);
}

Bitmap stamp_raster_selection(const Image& image){
  const RasterSelection& selection = image.GetRasterSelection();

  return image.GetBackground().Visit(
    [&](const Bitmap& bmp){
      Bitmap copy(bmp);
      FaintDC dc(copy);
      selection.DrawFloating(dc);
      return copy;
    },
    [&](const ColorSpan& span){
      Bitmap bmp(span.size, span.color);
      FaintDC dc(bmp);
      selection.DrawFloating(dc);
      return bmp;
    });
}

Bitmap subbitmap(const Image& image, const IntRect& rect){
  return image.GetBackground().Visit(
    [&](const Bitmap& bmp){
      return subbitmap(bmp, rect);
    },
    [&](const ColorSpan& span){
      return Bitmap(rect.GetSize(), span.color);
    });
}

static void flatten_onto(Bitmap& bmp,
  const objects_t& objects,
  const RasterSelection& selection,
  ExpressionContext& ctx)
{
  FaintDC dc(bmp);
  selection.DrawFloating(dc);
  for (Object* obj : objects){
    obj->Draw(dc, ctx);
  }
}

Bitmap flatten(const Image& image){
  const objects_t& objects = image.GetObjects();
  const RasterSelection& selection = image.GetRasterSelection();
  return image.GetBackground().Visit(
    [&](const Bitmap& bmp){
      Bitmap copy(bmp);
      flatten_onto(copy, objects, selection, image.GetExpressionContext());
      return copy;
    },
    [&](const ColorSpan& span){
      Bitmap bmp(span.size, span.color);
      flatten_onto(bmp, objects, selection, image.GetExpressionContext());
      return bmp;
    });
}

int get_highest_z(const Image& image){
  int numObjects = image.GetNumObjects();
  assert(numObjects != 0);
  return numObjects - 1;
}

bool has_all(const Image& image, const objects_t& objects){
  for (const Object* obj : objects){
    if (!image.Has(obj)){
      return false;
    }
  }
  return true;
}

bool point_in_image(const Image& image, const IntPoint& pos){
  IntSize size(image.GetSize());
  return pos.x >= 0 && pos.y >= 0 && pos.x < size.w && pos.y < size.h;
}

void remove_missing_objects_from(objects_t& objects, const Image& image){
  auto object_not_in_image = [&image](const Object* obj){
    return !image.Has(obj);
  };
  objects.erase(remove_if(begin(objects), end(objects), object_not_in_image),
    end(objects));
}

Optional<coord> get_pixel_to_mm_conversion(const Image& image){
  return image.GetCalibration().Visit(
    [](const Calibration& c) -> Optional<coord>{
      return c.unit == "mm" ?
        option(c.length / length(c.pixelLine)) :
        no_option();
    },
    []() -> Optional<coord>{
      return no_option();
    });
}

} // namespace faint
