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

#include "bitmap/color.hh"
#include "bitmap/gradient.hh"
#include "bitmap/paint.hh"
#include "bitmap/pattern.hh"
#include "util/optional.hh"

namespace faint{

class Paint::PaintImpl{
public:
  PaintImpl(const Color& color)
    : color(color)
  {}
  PaintImpl(const Pattern& pattern)
    : pattern(pattern)
  {}
  PaintImpl(const Gradient& gradient)
    : gradient(gradient)
  {}
  Optional<Color> color;
  Optional<Pattern> pattern;
  Optional<Gradient> gradient;
};

Paint::Paint(){
  m_impl = new PaintImpl(color_black);
}

Paint::Paint(const Color& color){
  m_impl = new PaintImpl(color);
}

Paint::Paint(const Gradient& g){
  m_impl = new PaintImpl(g);
}

Paint::Paint(const Pattern& pattern){
  m_impl = new PaintImpl(pattern);
}

Paint::Paint(const Paint& other){
  m_impl = new PaintImpl(*other.m_impl);
}

Paint::Paint(Paint&& other){
  m_impl = other.m_impl;
  other.m_impl = nullptr;
}

Paint::~Paint(){
  delete m_impl;
}

bool Paint::IsColor() const{
  return m_impl->color.IsSet();
}

bool Paint::IsGradient() const{
  return m_impl->gradient.IsSet();
}

bool Paint::IsPattern() const{
  return m_impl->pattern.IsSet();
}

const Color& Paint::GetColor() const {
  return m_impl->color.Get();
}

const Gradient& Paint::GetGradient() const{
  return m_impl->gradient.Get();
}

const Pattern& Paint::GetPattern() const {
  return m_impl->pattern.Get();
}

Paint& Paint::operator=(const Paint& other){
  if (this == &other){
    return *this;
  }

  delete m_impl;
  m_impl = new PaintImpl(*other.m_impl);
  return *this;
}

bool operator==(const Paint& a, const Paint& b){
  if (a.IsColor()){
    return b.IsColor() ?
      a.GetColor() == b.GetColor() :
      false;
  }
  else if (a.IsGradient()){
    return b.IsGradient() ?
      a.GetGradient() == b.GetGradient() :
      false;
  }
  else if (a.IsPattern()){
    return b.IsPattern() ?
      a.GetPattern() == b.GetPattern() :
      false;
  }
  assert(false);
  return false;
}

bool operator!=(const Paint& a, const Paint& b){
  return !operator==(a,b);
}

bool operator==(const Paint& paint, const Color& c){
  return paint.IsColor() && paint.GetColor() == c;
}

bool operator==(const Color& c, const Paint& paint){
  return paint.IsColor() && paint.GetColor() == c;
}

bool operator!=(const Paint& paint, const Color& c){
  return !operator==(paint, c);
}

bool operator!=(const Color& c, const Paint& paint){
  return !operator==(paint, c);
}

Paint offsat(const Paint& paint, const IntPoint& delta){
  if (paint.IsPattern()){
    return Paint(offsat(paint.GetPattern(), delta));
  }
  return paint;
}

Paint offsat(const Paint& paint,
  const IntPoint& delta,
  const IntPoint& clickPos)
{
  if (paint.IsPattern()){
    Pattern p(paint.GetPattern());
    bool useClickPos = p.GetObjectAligned();
    p.SetAnchor(p.GetAnchor() + delta -
      (useClickPos ? clickPos : IntPoint(0,0)));
    return Paint(p);
  }
  return paint;
}

Color get_color_default(const Paint& paint, const Color& defaultColor){
  return paint.IsColor() ?
    paint.GetColor() :
    defaultColor;
}

bool is_opaque_color(const Paint& paint){
  return paint.IsColor() && opaque(paint.GetColor());
}

bool is_translucent_color(const Paint& paint){
  return paint.IsColor() && translucent(paint.GetColor());
}

} // namespace
