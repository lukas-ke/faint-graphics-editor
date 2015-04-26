#ifndef FAINT_COLOR_PTR_HH
#define FAINT_COLOR_PTR_HH
#include "bitmap/color.hh"

namespace faint{

class color_ptr{
public:
  // For fetching rgba values from an uchar-pointer
  // (Which must be at the correct four-byte offset)
  uchar& r;
  uchar& g;
  uchar& b;
  uchar& a;
  color_ptr(uchar* p):
    r(*(p + iR)),
    g(*(p + iG)),
    b(*(p + iB)),
    a(*(p + iA))
  {}
  void Set(const Color& c){
    r = c.r;
    g = c.g;
    b = c.b;
    a = c.a;
  }
  color_ptr& operator=(const color_ptr&) = delete;
};

class const_color_ptr{
public:
  // For fetching rgba values from an uchar-pointer
  // (Which must be at the correct four-byte offset)
  const uchar r;
  const uchar g;
  const uchar b;
  const uchar a;
  const_color_ptr(const uchar* p):
    r(*(p + iR)),
    g(*(p + iG)),
    b(*(p + iB)),
    a(*(p + iA))
  {}

  const_color_ptr& operator=(const const_color_ptr&) = delete;
};

inline bool operator==(const color_ptr& c1, const Color& c2){
  return c1.r == c2.r &&
    c1.g == c2.g &&
    c1.b == c2.b &&
    c1.a == c2.a;
}

} // namespace

#endif
