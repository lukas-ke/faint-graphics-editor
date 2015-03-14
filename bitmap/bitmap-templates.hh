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

#ifndef FAINT_BITMAP_TEMPLATES_HH
#define FAINT_BITMAP_TEMPLATES_HH
#include <functional>
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "geo/int-rect.hh"
#include "rendering/cairo-context.hh" // FIXME

namespace faint{

template<typename T>
Bitmap create_bitmap(const IntSize& sz, const T& func){
  Bitmap bmp(sz);
  func(bmp);
  return bmp;
}

template<typename T>
Bitmap create_bitmap(const IntSize& sz, const Color& c, const T& func){
  Bitmap bmp(sz, c);
  func(bmp);
  return bmp;
}

template<typename T>
Bitmap create_bitmap(const Bitmap& src, const T& func){
  Bitmap bmp(src);
  func(bmp);
  return bmp;
}

inline int wrap(int v, int n){
  return v >= 0 ?
    v % n :
    (std::abs(v) * (n - 1)) % n;
}

inline Color get_color_modulo_raw(const Bitmap& bmp, int x, int y){
  return get_color_raw(bmp, wrap(x, bmp.m_w), wrap(y, bmp.m_h));
}

inline Color get_color_modulo(const Bitmap& bmp, const IntPoint& pos){
  return get_color_raw(bmp, wrap(pos.x, bmp.m_w), wrap(pos.y, bmp.m_h));
}


struct ColorFromColor{
  ColorFromColor(const Color& color)
    : m_color(color)
  {}
  void operator()(Bitmap& dst, int x, int y) const{
    put_pixel_raw(dst, x, y, m_color);
  }
  Color m_color;
};

struct ColorFromGradient{
  // Functor for using a Gradient as draw source
  // Fixme: Use new ColorFromBitmap with both Pattern and Gradient
  ColorFromGradient(const Gradient& g, const IntRect& r)
    : m_bmp(cairo_gradient_bitmap(g, r.GetSize())),
      m_x(-r.x),
      m_y(-r.y)
  {}
  void operator()(Bitmap& dst, int x, int y) const{
    // Fixme: Blend or set should depend on ts_AlphaBlending
    put_pixel_raw(dst, x, y, get_color_modulo_raw(m_bmp, x + m_x, y + m_y));
  }

  void operator()(Bitmap& dst, int x, int y, uchar a) const{
    // Fixme: Blend or set should depend on ts_AlphaBlending
    Color c(strip_alpha(get_color_modulo_raw(m_bmp, x + m_x, y + m_y)), a);
    put_pixel_raw(dst, x, y, c);
  }

  Bitmap m_bmp;
  int m_x;
  int m_y;
private:
  ColorFromGradient& operator=(const ColorFromGradient&); // Silence warning
};

struct ColorFromPattern{
  // Functor for using a bitmap as draw source
  ColorFromPattern(const Pattern& pattern)
    : m_bmp(pattern.GetBitmap())
  {
    const IntPoint offset = pattern.GetAnchor();
    m_x = offset.x;
    m_y = offset.y;
  }
  void operator()(Bitmap& dst, int x, int y) const{
    // Fixme: Blend or Set should depend on ts_AlphaBlending
    put_pixel_raw(dst, x, y, get_color_modulo_raw(m_bmp, x + m_x, y + m_y));
  }

  void operator()(Bitmap& dst, int x, int y, uchar a) const{
    // Fixme: Blend or Set should depend on ts_AlphaBlending
    Color c(strip_alpha(get_color_modulo_raw(m_bmp, x + m_x, y + m_y)), a);
    put_pixel_raw(dst, x, y, c);
}
  const Bitmap& m_bmp;
  int m_x;
  int m_y;
private:
  ColorFromPattern& operator=(const ColorFromPattern&); // Silence warning
};

template<typename Functor1, typename Functor2, typename Condition>
void set_pixels_if_else_f(Bitmap& bmp,
  const Functor1& setPixFunc1,
  const Functor2& setPixFunc2,
  const Condition& condition)
{
  IntSize sz(bmp.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      if (condition(x,y)){
        setPixFunc1(bmp, x, y);
      }
      else{
        setPixFunc2(bmp, x, y);
      }
    }
  }
}

template<typename Condition>
void set_pixels_if_else(Bitmap& bmp,
  const Paint& onTrue,
  const Paint& onFalse,
  const Condition& condition)
{
  if (onTrue.IsColor()){
    if (onFalse.IsColor()){
    set_pixels_if_else_f(bmp, ColorFromColor(onTrue.GetColor()),
      ColorFromColor(onFalse.GetColor()), condition);
    }
    else if (onFalse.IsPattern()){
      set_pixels_if_else_f(bmp, ColorFromColor(onTrue.GetColor()),
        ColorFromPattern(onFalse.GetPattern()), condition);
    }
    else if (onFalse.IsGradient()){
      set_pixels_if_else_f(bmp, ColorFromColor(onTrue.GetColor()),
        ColorFromGradient(onFalse.GetGradient(),
          IntRect(IntPoint(0,0), bmp.GetSize())), condition);
    }
  }
  else if (onTrue.IsPattern()){
    if (onFalse.IsColor()){
      set_pixels_if_else_f(bmp, ColorFromPattern(onTrue.GetPattern()),
        ColorFromColor(onFalse.GetColor()), condition);
    }
    else if (onFalse.IsPattern()){
      set_pixels_if_else_f(bmp, ColorFromPattern(onTrue.GetPattern()),
        ColorFromPattern(onFalse.GetPattern()), condition);
    }
    else if (onFalse.IsGradient()){
      set_pixels_if_else_f(bmp, ColorFromPattern(onTrue.GetPattern()),
        ColorFromGradient(onFalse.GetGradient(),
          IntRect(IntPoint(0,0), bmp.GetSize())), condition);
    }
  }
  else if (onTrue.IsGradient()){
    ColorFromGradient lhs(onTrue.GetGradient(),
      IntRect(IntPoint(0,0), bmp.GetSize()));
    if (onFalse.IsColor()){
      set_pixels_if_else_f(bmp, lhs,
        ColorFromColor(onFalse.GetColor()), condition);
    }
    else if (onFalse.IsPattern()){
      set_pixels_if_else_f(bmp, lhs,
        ColorFromPattern(onFalse.GetPattern()), condition);
    }
    else if (onFalse.IsGradient()){
      set_pixels_if_else_f(bmp, lhs,
        ColorFromGradient(onFalse.GetGradient(),
          IntRect(IntPoint(0,0), bmp.GetSize())), condition);
    }
  }
}

template<typename Functor, typename Condition>
void set_pixels_if(Bitmap& bmp,
  const Functor& setPixFunc,
  const Condition& condition)
{
  IntSize sz(bmp.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      if (condition(x,y)){
        setPixFunc(bmp, x, y);
      }
    }
  }
}

template<typename SetFunctor, typename BlendFunctor>
void blend_pixels(Bitmap& bmp,
  const SetFunctor& setPixFunc,
  const BlendFunctor& blendFunc)
{
  IntSize sz(bmp.GetSize());
  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      setPixFunc(bmp, x, y, blendFunc(x,y));
    }
  }
}

// Forwarder which applies an, otherwise in-place function, to a copy of
// the Bitmap parameter and returns the result.
template<typename ...Args1, typename ...Args2>
Bitmap onto_new(void(*func)(Bitmap&, Args1...),
  const Bitmap& src,
  const Args2&... args)
{
  Bitmap bmp(src);
  func(bmp, args...);
  return bmp;
}

} // namespace

#endif
