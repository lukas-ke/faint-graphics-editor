// -*- coding: us-ascii-unix -*-
#ifndef FAINT_TEST_UTIL_PRINT_OBJECTS_HH
#define FAINT_TEST_UTIL_PRINT_OBJECTS_HH
#include <iosfwd>

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

} // namespace

#endif
