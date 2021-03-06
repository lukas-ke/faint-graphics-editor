// -*- coding: us-ascii-unix -*
#ifndef FAINT_TEST_BITMAP_TEST_UTIL_HH
#define FAINT_TEST_BITMAP_TEST_UTIL_HH

namespace faint{

class Color;
class Bitmap;
class IntRect;

bool is_uniformly(const Color&, const Bitmap&);
bool is_uniformly(const Color&, const Bitmap&, const IntRect&);
bool equal(const Bitmap&, const Bitmap&);

} // namespace

#endif
