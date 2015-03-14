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
#include "bitmap/bitmap-templates.hh"
#include "bitmap/color.hh"
#include "bitmap/filter.hh"
#include "bitmap/gaussian-blur.hh"
#include "geo/angle.hh"
#include "geo/padding.hh"
#include "geo/point.hh"
#include "rendering/filter-class.hh"

namespace faint{

template<int PADDING, void Func(Bitmap&)>
class FunctionFilter : public Filter{
public:
  void Apply(Bitmap& bmp) const override{
    Func(bmp);
  }

  Padding GetPadding() const override{
    return Padding::All(PADDING);
  }
};

class FilterStroke : public Filter{
public:
  void Apply(Bitmap& bmp) const override{
    // Fixme: Allocation, handle OOM
    Bitmap bg(bmp.GetSize(), color_transparent_white);
    for (int y = 1; y < bmp.m_h - 1; y++){
      for (int x = 1; x < bmp.m_w - 1; x++){
        Color current = get_color_raw(bmp, x, y);
        Color right(get_color_raw(bmp, x + 1, y));
        if (current.a == 0 && right.a != 0){
          fill_ellipse_color(bg,
            IntRect(IntPoint(x - 3, y - 3), IntSize(7,7)),
            Color(0,0,0));
        }
        else if (current.a != 0 && right.a == 0){
          fill_ellipse_color(bg,
            IntRect(IntPoint(x - 3 + 1, y - 3), IntSize(7,7)),
            Color(0,0,0));
        }

        Color down(get_color_raw(bmp, x, y + 1));
        if (current.a == 0 && down.a != 0){
          fill_ellipse_color(bg,
            IntRect(IntPoint(x - 3, y - 3), IntSize(7,7)),
            Color(0,0,0));
        }
        if (current.a != 0 && down.a == 0){
          fill_ellipse_color(bg,
            IntRect(IntPoint(x - 3, y - 3 + 1), IntSize(7,7)),
            Color(0,0,0));
        }
      }
    }
    blit_masked(at_top_left(bmp), onto(bg), color_transparent_white);
    bmp = bg;
  }

  Padding GetPadding() const override{
    return Padding::All(5);
  }
};

static void increase_alpha(Bitmap& bmp, int x, int y, int d){
  Color c = get_color_raw(bmp, x, y);
  c.a = static_cast<uchar>(std::min(static_cast<int>(c.a) + d, 255));
  put_pixel_raw(bmp, x, y, c);
}

class FilterShadow : public Filter{
public:
  int m_dist;
  FilterShadow(){
    m_dist = 9;
  }
  void Apply(Bitmap& bmp) const override{
    // Fixme: Allocation, handle OOM
    Bitmap bg(bmp.GetSize(), color_transparent_black);
    const int c1 = 20;
    const int c2 = 15;
    const int c3 = 10;
    const int c4 = 5;
    for (int y = 0; y < bmp.m_h - m_dist - 5; y++){
      for (int x = 0; x < bmp.m_w - m_dist - 5; x++){
        if (get_color_raw(bmp, x, y).a != 0){
          int x2 = x + m_dist;
          int y2 = y + m_dist;
          increase_alpha(bg, x2 + 1, y2, c4);
          increase_alpha(bg, x2 + 2, y2, c3);
          increase_alpha(bg, x2 + 3, y2, c4);

          increase_alpha(bg, x2, y2 + 1, c4);
          increase_alpha(bg, x2 + 1, y2 + 1, c3);
          increase_alpha(bg, x2 + 2, y2 + 1, c2);
          increase_alpha(bg, x2 + 3, y2 + 1, c3);
          increase_alpha(bg, x2 + 4, y2 + 1, c4);

          increase_alpha(bg, x2, y2 + 2, c3);
          increase_alpha(bg, x2 + 1, y2 + 2, c2);
          increase_alpha(bg, x2 + 2, y2 + 2, c1);
          increase_alpha(bg, x2 + 3, y2 + 2, c2);
          increase_alpha(bg, x2 + 4, y2 + 2, c3);

          increase_alpha(bg, x2, y2 + 3, c4);
          increase_alpha(bg, x2 + 1, y2 + 3, c3);
          increase_alpha(bg, x2 + 2, y2 + 3, c2);
          increase_alpha(bg, x2 + 3, y2 + 3, c3);
          increase_alpha(bg, x2 + 4, y2 + 3, c4);

          increase_alpha(bg, x2 + 1, y2 + 4, c4);
          increase_alpha(bg, x2 + 2, y2 + 4, c3);
          increase_alpha(bg, x2 + 3, y2 + 4, c4);
        }
      }
    }

    blit_masked(at_top_left(bmp), onto(bg), color_transparent_white);
    bmp = bg;
  }

  Padding GetPadding() const override{
    return Padding::Right(6 + m_dist) + Padding::Bottom(6 + m_dist);
  }
};

Bitmap brightness_and_contrast(const Bitmap& src, const brightness_contrast_t& v){
  double scaledBrightness = v.brightness * 255.0;
  Bitmap dst(src.GetSize());

  for (int y = 0; y != src.m_h; y++){
    for (int x = 0; x != src.m_w; x++){
      const auto c = get_color_raw(src, x, y);
      const auto c2 =
        color_from_double(c.r * v.contrast + scaledBrightness,
          c.g * v.contrast + scaledBrightness,
          c.b * v.contrast + scaledBrightness,
          c.a);
      put_pixel_raw(dst, x, y, c2);
    }
  }
  return dst;
}

void desaturate_simple(Bitmap& bmp){
  uchar* data = bmp.m_data;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      int dst = y * bmp.m_row_stride + x * BPP;
      uchar gray = static_cast<uchar>((data[dst + iR] +
          data[dst + iG] +
          data[dst + iB]) / 3);
      data[dst + iR] = gray;
      data[dst + iG] = gray;
      data[dst + iB] = gray;
    }
  }
}

void desaturate_weighted(Bitmap& bmp){
  uchar* data = bmp.m_data;
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      int dst = y * bmp.m_row_stride + x * BPP;
      uchar gray = static_cast<uchar>(0.3 * data[dst + iR] +
          0.59 * data[dst + iG] +
          0.11 * data[dst + iB]);
      data[dst + iR] = gray;
      data[dst + iG] = gray;
      data[dst + iB] = gray;
    }
  }
}

void pixelize(Bitmap& bmp, const pixelize_range_t& width){
  static_assert(pixelize_range_t::min_allowed > 0,
    "pixelize expects min bound > 0");
  int w(width.GetValue());

  IntSize sz(bmp.GetSize());
  Bitmap bmp2(sz);
  for (int y = 0; y < sz.h; y +=w){
    for (int x = 0; x < sz.w; x +=w){
      int r = 0;
      int g = 0;
      int b = 0;
      int a = 0;
      int count = 0;
      for (int j = 0; y + j != sz.h && j != w; j++){
        for (int i = 0; x + i != sz.w && i != w; i++){
          Color c = get_color_raw(bmp, x + i, y + j);
          r += c.r;
          g += c.g;
          b += c.b;
          a += c.a;
          count++;
        }
      }

      Color c2(color_from_ints(r / count, g / count, b / count, a / count));
      for (int j = 0; y + j < sz.h && j != w; j++){
        for (int i = 0; x + i != sz.w && i != w; i++){
          put_pixel_raw(bmp2, x + i, y + j, c2);
        }
      }
    }
  }
  bmp = bmp2;
}

void sepia(Bitmap& bmp, int intensity){
  // Modified from:
  // https://groups.google.com/forum/#!topic/comp.lang.java.programmer/nSCnLECxGdA
  int depth = 20;
  const IntSize sz = bmp.GetSize();

  for (int y = 0; y != sz.h; y++){
    for (int x = 0; x != sz.w; x++){
      Color c = get_color_raw(bmp, x, y);
      const int gray = (c.r + c.g + c.b) / 3;
      const int r = std::min(gray + depth * 2, 255);
      const int g = std::min(gray + depth, 255);
      const int b = std::max(gray - intensity, 0);
      put_pixel_raw(bmp, x, y, color_from_ints(r,g,b, c.a));
    }
  }
}

static int color_sum(const Color& c){
  return c.r + c.g + c.b;
}

void threshold(Bitmap& bmp, const threshold_range_t& range, const Paint& in, const Paint& out){
  const Interval interval(range.GetInterval());
  set_pixels_if_else(bmp, in, out,
    [&interval, &bmp](int x, int y){
      return interval.Has(color_sum(get_color_raw(bmp, x, y)));
    });
}

std::vector<int> threshold_histogram(const Bitmap& bmp){
  std::vector<int> v(766, 0);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, y));
      v[c.r + c.g + c.b] += 1;
    }
  }
  return v;
}

Bitmap subtract(const Bitmap& lhs, const Bitmap& rhs){
  assert(lhs.GetSize() == rhs.GetSize());
  Bitmap dst(lhs.GetSize());
  for (int y = 0; y != lhs.m_h; y++){
    for (int x = 0; x != lhs.m_w; x++){
      put_pixel_raw(dst, x, y,
        subtract(get_color_raw(lhs, x, y), get_color_raw(rhs, x, y)));
    }
  }
  return dst;
}

Bitmap add(const Bitmap& lhs, const Bitmap& rhs){
  assert(lhs.GetSize() == rhs.GetSize());
  Bitmap dst(lhs.GetSize());
  for (int y = 0; y != lhs.m_h; y++){
    for (int x = 0; x != lhs.m_w; x++){
      put_pixel_raw(dst, x, y,
        add(get_color_raw(lhs, x, y), get_color_raw(rhs, x, y)));
    }
  }
  return dst;
}

Bitmap unsharp_mask_fast(const Bitmap& bmp, double blurSigma){
  Bitmap blurred = gaussian_blur_fast(bmp, blurSigma);
  Bitmap usm = subtract(bmp, blurred);
  return add(bmp, usm);
}

Bitmap unsharp_mask_exact(const Bitmap& bmp, double blurSigma){
  Bitmap blurred = gaussian_blur_exact(bmp, blurSigma);
  Bitmap usm = subtract(bmp, blurred);
  return add(bmp, usm);
}

std::vector<int> red_histogram(Bitmap& bmp){
  std::vector<int> v(256,0);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, y));
      v[c.r] += 1;
    }
  }
  return v;
}

std::vector<int> green_histogram(Bitmap& bmp){
  std::vector<int> v(256,0);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, y));
      v[c.g] += 1;
    }
  }
  return v;
}

std::vector<int> blue_histogram(Bitmap& bmp){
  std::vector<int> v(256,0);
  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Color c(get_color_raw(bmp, x, y));
      v[c.b] += 1;
    }
  }
  return v;
}

Filter* get_invert_filter(){
  return new FunctionFilter<0, invert>();
}

Filter* get_stroke_filter(){
  return new FilterStroke();
}

Filter* get_shadow_filter(){
  return new FilterShadow();
}

void pixelize_5(Bitmap& bmp){
  pixelize(bmp, 5);
}

Filter* get_pixelize_filter(){
  using namespace std::placeholders;
  return new FunctionFilter<0, pixelize_5>();
}

void invert(Bitmap& bmp){
  for (int y = 0; y != bmp.m_h; y++){
    uchar* data = bmp.m_data + y * bmp.m_row_stride;
    for (int x = 0; x != bmp.m_w * BPP; x += BPP){
      uchar* pos = data + x;
      *(pos + iR) = static_cast<uchar>(255 - *(pos + iR));
      *(pos + iG) = static_cast<uchar>(255 - *(pos + iG));
      *(pos + iB) = static_cast<uchar>(255 - *(pos + iB));
    }
  }
}

static uchar clip_rgb(coord value){
  return static_cast<uchar>(std::min(255.0, std::max(0.0, value)));
}

void color_balance(Bitmap& bmp,
  const color_range_t& r,
  const color_range_t& g,
  const color_range_t& b)
{
  const Interval rSpan(r.GetInterval());
  const Interval gSpan(b.GetInterval());
  const Interval bSpan(g.GetInterval());
  double Ar = static_cast<coord>(rSpan.Lower());
  double Br = static_cast<coord>(rSpan.Upper());
  double Ag = static_cast<coord>(gSpan.Lower());
  double Bg = static_cast<coord>(gSpan.Upper());
  double Ab = static_cast<coord>(bSpan.Lower());
  double Bb = static_cast<coord>(bSpan.Upper());
  double L = 0.0;
  double U = 255.0;
  double Xr = (U-L) / (Br-Ar);
  double Yr = L - Ar*((U-L)/ (Br-Ar));
  double Xg = (U-L) / (Bg-Ag);
  double Yg = L - Ag*((U-L)/ (Bg-Ag));
  double Xb = (U-L) / (Bb-Ab);
  double Yb = L - Ab*((U-L)/ (Bb-Ab));

  for (int y = 0; y != bmp.m_h; y++){
    uchar* row = bmp.m_data + y * bmp.m_row_stride;
    for (int x = 0; x != bmp.m_w * BPP; x += BPP){
      uchar* pos = row + x;
      *(pos + iR) = clip_rgb(*(pos + iR) * Xr + Yr);
      *(pos + iG) = clip_rgb(*(pos + iG) * Xg + Yg);
      *(pos + iB) = clip_rgb(*(pos + iB) * Xb + Yb);
    }
  }
}

template<typename T>
T sq(T v){
  return v*v;
}

static IntPoint whirled(const Point& p, const Point& c, coord r,
  const Angle& whirl)
{
  const Angle angle = whirl * sq(1.0 - r);
  coord sina = sin(angle);
  coord cosa = cos(angle);
  return IntPoint(truncated(cosa * p.x - sina * p.y + c.x),
    truncated(sina * p.x + cosa * p.y + c.y));
}

void filter_pinch_whirl(Bitmap& bmp, coord pinch, const Angle& whirl){
  Bitmap bmpOld(bmp);
  Point c((bmp.m_w - 1) / 2.0, (bmp.m_h - 1) / 2.0);
  coord r2 = sq(bmp.m_w / 2.0);

  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      Point delta(x - c.x, y - c.y);
      coord d = sq(delta.x) + sq(delta.y);

      if (d < r2){
        coord dist = sqrt(d / 1.0) / (bmp.m_w / 2.0);
        delta *= pow(sin(pi/2*dist), pinch);
        IntPoint p2 = whirled(delta, c, dist, whirl);
        if (p2.x > 0 && p2.y > 0 && p2.x < bmp.m_w && p2.y < bmp.m_h){
          put_pixel_raw(bmp, x, y, get_color(bmpOld, p2));
        }
      }
    }
  }
}

void filter_pinch_whirl_forward(Bitmap& bmp){
  filter_pinch_whirl(bmp, 0.5, Angle::Rad(0.5));
}

Filter* get_pinch_whirl_filter(){
  return new FunctionFilter<10, filter_pinch_whirl_forward>();
}

} // namespace
