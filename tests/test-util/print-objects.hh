// -*- coding: us-ascii-unix -*-
#ifndef FAINT_TEST_PRINT_OBJECTS_HH
#define FAINT_TEST_PRINT_OBJECTS_HH
#include <iosfwd>
#include <vector>
#define PRINTER(CLASS)class CLASS; std::ostream& operator<<(std::ostream&, const CLASS&)

namespace faint{

PRINTER(Angle);
PRINTER(IntLineSegment);
PRINTER(IntPoint);
PRINTER(IntRect);
PRINTER(IntSize);
PRINTER(LineSegment);
PRINTER(Paint);
PRINTER(Point);
PRINTER(utf8_char);
PRINTER(Color);
PRINTER(ColRGB);
PRINTER(Index);

// Avoids unsigned char values being casted to char and printed as
// garbage ascii, by outputting them as unsigned int instead.
std::ostream& operator<<(std::ostream&, unsigned char);

template<typename T>
std::ostream& operator<<(std::ostream& o, const std::vector<T>& v){
  for (const auto& i : v){
    o << i << " ";
  }
  return o;
}

} // namespace

#endif
