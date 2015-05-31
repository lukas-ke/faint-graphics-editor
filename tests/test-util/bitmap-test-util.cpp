#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/iter-bmp.hh"
#include "tests/test-util/bitmap-test-util.hh"

namespace faint{

bool is_uniformly(const Color& c, const Bitmap& bmp){
  for (ITER_XY(x, y, bmp)){
    if (get_color_raw(bmp, x,y) != c){
      return false;
    }
  }
  return true;
}

bool is_uniformly(const Color& c, const Bitmap& bmp, const IntRect& r){
  return is_uniformly(c, subbitmap(bmp, r));
}

bool equal(const Bitmap& b1, const Bitmap& b2){
  if (b1.GetSize() != b2.GetSize()){
    return false;
  }

  for (ITER_XY(x, y, b1)){
    if (get_color_raw(b1, x, y) != get_color_raw(b2, x, y)){
      return false;
    }
  }
  return true;
}

} // namespace
