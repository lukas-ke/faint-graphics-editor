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

#include "app/cut-and-paste.hh"
#include "app/get-app-context.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "bitmap/filter.hh"
#include "geo/int-rect.hh"
#include "python/py-function-error.hh"
#include "text/formatting.hh"
#include "util/color-span.hh"
#include "util/either.hh"
#include "util-wx/clipboard.hh"

namespace faint{

void copy_rect_to_clipboard(const Bitmap& bmp, const IntRect& rect){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw OSError("Failed opening clipboard");
  }
  if (!fully_inside(rect, bmp)){
    throw ValueError("Rectangle not fully inside image.");
  }

  try{
    bitmap_to_clipboard(get_app_context(), clipboard,
      subbitmap(bmp, rect));
  }
  catch(const std::bad_alloc&){
    throw MemoryError("Insufficient memory for Bitmap copy.");
  }
}

static void copy_rect_to_clipboard(const ColorSpan& span, const IntRect& rect){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw OSError("Failed opening clipboard");
  }
  if (!fully_inside(rect, span)){
    throw ValueError("Rectangle not fully inside image.");
  }

  try{
    bitmap_to_clipboard(get_app_context(), clipboard,
      Bitmap(rect.GetSize(), span.color));
  }
  catch(const std::bad_alloc&){
    throw MemoryError("Insufficient memory for Bitmap copy.");
  }
}

void copy_rect_to_clipboard(const Either<Bitmap, ColorSpan>& src,
  const IntRect& rect)
{
  src.Visit(
    [&](const Bitmap& bmp){
      copy_rect_to_clipboard(bmp, rect);
    },
    [&](const ColorSpan& span){
      copy_rect_to_clipboard(span, rect);
    });
}

} // namespace
