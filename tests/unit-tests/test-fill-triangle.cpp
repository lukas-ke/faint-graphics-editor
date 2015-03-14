// -*- coding: us-ascii-unix -*-
#include <algorithm>
#include <array>
#include "test-sys/test.hh"
#include "tests/test-util/text-bitmap.hh"

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"

using faint::IntPoint;

using triangle_t = std::array<IntPoint, 3>;
using triangles_t = std::vector<triangle_t>;

static triangles_t permutations(const IntPoint& p0,
  const IntPoint& p1,
  const IntPoint& p2)
{
  triangle_t a = {{p0, p1, p2}};
  std::sort(begin(a), end(a));

  triangles_t v;
  do{
    v.push_back(a);
  } while (std::next_permutation(begin(a), end(a)));

  EQUAL(v.size(), 3*2*1);
  return v;
}

inline void fill_triangle_color(faint::Bitmap& bmp,
  const triangle_t& pts,
  const faint::Color& c)
{
  // Forward to the real function-under-test, to avoid repeating
  // point-retrieval in each test.
  using std::get;
  fill_triangle_color(bmp, get<0>(pts), get<1>(pts), get<2>(pts), c);
}

void test_fill_triangle(){
  using namespace faint;

  const bitmap_value_map colors = {
    {'.', color_white},
    {'X', color_black}};

  for (auto& pts : permutations({0,3}, {3,0}, {6,3})){
    Bitmap bmp({7, 4}, color_white);
    fill_triangle_color(bmp, pts, color_black);
    FWD(check(bmp,
      "...X..."
      "..XXX.."
      ".XXXXX."
      "XXXXXXX",
      colors));
  }

  for (auto& pts : permutations({0, 0}, {3, 3}, {0, 6})){
    Bitmap bmp({4, 7}, color_white);
    fill_triangle_color(bmp, pts, color_black);
    FWD(check(bmp,
      "X..."
      "XX.."
      "XXX."
      "XXXX"
      "XXX."
      "XX.."
      "X...",
      colors));
  }

  for (auto& pts : permutations({3, 0}, {0, 3}, {3, 6})){
    Bitmap bmp({4, 7}, color_white);
    fill_triangle_color(bmp, pts, color_black);
    FWD(check(bmp,
      "...X"
      "..XX"
      ".XXX"
      "XXXX"
      ".XXX"
      "..XX"
      "...X",
      colors));
  }

  for (auto& pts : permutations({0, 0}, {3, 0}, {0, 3})){
    Bitmap bmp({4, 4}, color_white);
    fill_triangle_color(bmp, pts, color_black);
    FWD(check(bmp,
      "XXXX"
      "XXX."
      "XX.."
      "X...",
      colors))
  }

  for (auto& pts : permutations({3,0}, {3, 3}, {0, 3})){
    Bitmap bmp({4, 4}, color_white);
    fill_triangle_color(bmp, pts , color_black);
    FWD(check(bmp,
      "...X"
      "..XX"
      ".XXX"
      "XXXX",
      colors));
  }

  for (auto& pts : permutations({0,7}, {4, 0}, {6, 8})){
    Bitmap bmp({7, 9}, color_white);
    fill_triangle_color(bmp, pts, color_black);
    FWD(check(bmp,
     "....X.."
     "...XX.."
     "...XX.."
     "..XXXX."
     "..XXXX."
     ".XXXXX."
     ".XXXXX."
     "XXXXXXX"
     "....XXX",
     colors));
  }
}
