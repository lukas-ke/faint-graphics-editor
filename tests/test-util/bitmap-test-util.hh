// -*- coding: us-ascii-unix -*
#ifndef FAINT_BITMAP_TEST_UTIL_HH
#define FAINT_BITMAP_TEST_UTIL_HH

namespace faint{

class Color;
class Bitmap;
class IntRect;

bool is_uniformly(const Color&, const Bitmap&);
bool is_uniformly(const Color&, const Bitmap&, const IntRect&);
bool equal(const Bitmap&, const Bitmap&);
bool equal_ignore_transparent(const Bitmap&, 
  const Bitmap&);

} // namespace

#endif
