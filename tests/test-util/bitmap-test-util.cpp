#include "bitmap-test-util.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "bitmap/color.hh"
#include "bitmap/iter-bmp.hh"

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

bool equal_ignore_transparent(const Bitmap& b1, 
  const Bitmap& b2)
{
  if (b1.GetSize() != b2.GetSize()){
    return false;
  }

  for (ITER_XY(x, y, b1)){
    auto c1 = get_color_raw(b1, x, y);
    auto c2 = get_color_raw(b2, x, y);    
    if (c1 != c2 && (c1.a != 0 || c2.a != 0)){
      return false;
    }
  }
  return true;
}

} // namespace
