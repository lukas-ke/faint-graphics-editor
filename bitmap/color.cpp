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

#include <algorithm> // min, max
#include <cassert>
#include <cmath> // fabs
#include "bitmap/color.hh"
#include "geo/typed-range.hh"

namespace faint{

static const auto colorRange = TypedRange<uchar>(0, 255);

static uchar invert_color(uchar v){
  // gcc 4.8.1 with -Wconversion insisted on this cast.
  return static_cast<uchar>(255 - v);
}

template<typename T>
static uchar constrain_color(T v){
  return colorRange.Constrain(v);
}

bool ColRGB::operator==(const ColRGB& other) const{
  return r == other.r &&
    g == other.g &&
    b == other.b;
}

bool ColRGB::operator!=(const ColRGB& other) const{
  return !operator==(other);
}

bool Color::operator==(const Color& c2) const{
  return r == c2.r && g == c2.g && b == c2.b && a == c2.a;
}

bool Color::operator!=(const Color& c2) const{
  return r != c2.r || g != c2.g || b != c2.b || a != c2.a;
}

bool Color::operator<(const Color& c2) const{
  return to_hash(*this) < to_hash(c2);
}

bool Color::operator>(const Color& c2) const{
  return to_hash(*this) > to_hash(c2);
}

HSL::HSL(double h, double s, double l) :
  h(h),
  s(s),
  l(l)
{
  assert(0 <= h && h <= 360.0);
}

HSL::HSL(const HS& hueSat, double l)
  : h(hueSat.h),
    s(hueSat.s),
    l(l)
{}

HS HSL::GetHS() const{
  return {h,s};
}

static ColRGB rgb_from_hash(unsigned int h){
  auto r = static_cast<uchar>((h >> 16) & 0xff);
  auto g = static_cast<uchar>((h >> 8) & 0xff);
  auto b = static_cast<uchar>((h) & 0xff);
  return {r,g,b};
}

static unsigned int to_hash(const ColRGB& c){
  return (static_cast<unsigned int>(c.r) << 16) |
    (static_cast<unsigned int>(c.g) << 8) |
    (static_cast<unsigned int>(c.b));
}

bool ColRGB::operator<(const ColRGB& c2) const{
  return to_hash(*this) < to_hash(c2);
}

ColRGB blend_alpha(const Color& color, const ColRGB& bg){
  const auto a = color.a;
  const auto ia = invert_color(a);
  return rgb_from_ints(
    (color.r * a + bg.r * ia) / 255,
    (color.g * a + bg.g * ia) / 255,
    (color.b * a + bg.b * ia) / 255);
}

ColRGB strip_alpha(const Color& color){
  return ColRGB(color.r, color.g, color.b);
}

ColRGB invert(const ColRGB& c){
  return {invert_color(c.r), invert_color(c.g), invert_color(c.b)};
}

Color color_from_ints(int r, int g, int b, int a){
  return {
    constrain_color(r),
    constrain_color(g),
    constrain_color(b),
    constrain_color(a)};
}

ColRGB grayscale_rgb(int lightness){
  return rgb_from_ints(lightness, lightness, lightness);
}

Color grayscale_rgba(int lightness, int alpha){
  return color_from_ints(lightness, lightness, lightness, alpha);
}

ColRGB rgb_from_ints(int r, int g, int b){
  return {constrain_color(r), constrain_color(g), constrain_color(b)};
}

Color color_from_uints(uint r, uint g, uint b, uint a){
  return {
    constrain_color(r),
    constrain_color(g),
    constrain_color(b),
    constrain_color(a)};
}

Color color_from_double(double r, double g, double b, double a){
  return {constrain_color(r),
    constrain_color(g),
    constrain_color(b),
    constrain_color(a)};
}

ColRGB rgb_from_double(double r, double g, double b){
  return {
    constrain_color(r),
    constrain_color(g),
    constrain_color(b)};
}

bool opaque(const Color& c){
  return c.a == 255;
}

bool fully_transparent(const Color& c){
  return c.a == 0;
}

bool translucent(const Color& c){
  return c.a != 255;
}

bool valid_color(int r, int g, int b, int a){
  return has_all(colorRange, r, g, b, a);
}

bool valid_color(int r, int g, int b){
  return valid_color(r, g, b, 255);
}

Color with_alpha(const ColRGB& rgb, uchar a){
  return Color(rgb, a);
}

Color with_alpha(const Color& c, uchar a){
  return with_alpha(strip_alpha(c), a);
}

double dmod(double value, double range){
  while (value >= range){
    value -= range;
  }
  return value;
}

HSL to_hsl(const ColRGB& c){
  double R = c.r / 255.0;
  double G = c.g / 255.0;
  double B = c.b / 255.0;
  double M = std::max({R, G, B});
  double m = std::min({R, G, B});
  double C = M - m;
  double Hp = 0;

  if (C == 0){
    Hp = 0.0;
  }
  else if (M == R){
    Hp = ((((G - B) / C))) + 6.0;
  }
  else if (M == G){
    Hp = ((B - R) / C) + (2.0);
  }
  else if (M == B){
    Hp = ((R - G) / C) + (4.0);
  }

  // Hue
  double H = dmod(60.0 * Hp, 360.0);

  // Lightness
  // ...using Luma instead (i.e. weighted for perceived luminance
  // could be better, but trickier
  double L = (M + m) / 2.0;

  // Saturation
  double S = C == 0 ? 0 :
    (C / (1.0 - std::fabs(2 * L - 1.0)));
  return HSL(H, S, L);
}

ColRGB to_rgb(const HSL& c){
  double C = (1.0 - std::fabs(2 * c.l - 1.0)) * c.s;
  double Hp = c.h / 60.0;
  double X = C * (1 - std::fabs(dmod(Hp, 2.0) - 1.0));

  double R, G, B;
  if (Hp < 1){
    R = C;
    G = X;
    B = 0;
  }
  else if (Hp < 2){
    R = X;
    G = C;
    B = 0;
  }
  else if (Hp < 3){
    R = 0;
    G = C;
    B = X;
  }
  else if (Hp < 4){
    R = 0;
    G = X;
    B = C;
  }
  else if (Hp < 5){
    R = X;
    G = 0;
    B = C;
  }
  else if (Hp <= 6){
    R = C;
    G = 0;
    B = X;
  }
  else{
    assert(false);
    return ColRGB(0,0,0);
  }
  double m = c.l - C / 2.0;
  return rgb_from_double(255 * (R + m) + 0.5,
    255 * (G + m) + 0.5,
    255 * (B + m) + 0.5);
}

Color to_rgba(const HSL& hsl, int a){
  return Color(to_rgb(hsl),
    constrain_color(a));
}

Color subtract(const Color& lhs, const Color& rhs){
  int r = static_cast<int>(lhs.r) - static_cast<int>(rhs.r);
  int g = static_cast<int>(lhs.g) - static_cast<int>(rhs.g);
  int b = static_cast<int>(lhs.b) - static_cast<int>(rhs.b);
  int a = static_cast<int>(lhs.a) - static_cast<int>(rhs.a);
  return color_from_ints(r, g, b, a);
}

Color add(const Color& lhs, const Color& rhs){
  int r = static_cast<int>(lhs.r) + static_cast<int>(rhs.r);
  int g = static_cast<int>(lhs.g) + static_cast<int>(rhs.g);
  int b = static_cast<int>(lhs.b) + static_cast<int>(rhs.b);
  int a = static_cast<int>(lhs.a) + static_cast<int>(rhs.a);
  return color_from_ints(r, g, b, a);
}

int sum_rgb(const Color& c){
  return static_cast<int>(c.r) +
    static_cast<int>(c.g) +
    static_cast<int>(c.b);
}

unsigned int to_hash(const Color& c){
  return (static_cast<unsigned int>(c.r) << 24) |
    (static_cast<unsigned int>(c.g) << 16) |
    (static_cast<unsigned int>(c.b) << 8) |
    (static_cast<unsigned int>(c.a));
}

Color color_from_hash(unsigned int h){
  auto r = static_cast<uchar>((h >> 24) & 0xff);
  auto g = static_cast<uchar>((h >> 16) & 0xff);
  auto b = static_cast<uchar>((h >> 8) & 0xff);
  auto a = static_cast<uchar>((h & 0xff));
  return Color(r,g,b,a);
}

Color mix(const Color& c1, const Color& c2){
  return color_from_ints(
    (c1.r + c2.r) / 2,
    (c1.g + c2.g) / 2,
    (c1.b + c2.b) / 2,
    (c1.a + c2.a) / 2);
}

ColRGB next_color(const ColRGB& c){
  auto h = to_hash(c);
  return h == 0xffffff ? c :
    rgb_from_hash(h + 1);
}

Color color_from_hex(unsigned int v){
  auto r = (v >> 16) & 0xff;
  auto g = (v >> 8) & 0xff;
  auto b = v & 0xff;
  return color_from_ints(r, g, b);
}

ColRGB rgb_from_hex(unsigned int v){
  auto r = (v >> 16) & 0xff;
  auto g = (v >> 8) & 0xff;
  auto b = v & 0xff;
  return rgb_from_ints(r, g, b);
}

} // namespace
