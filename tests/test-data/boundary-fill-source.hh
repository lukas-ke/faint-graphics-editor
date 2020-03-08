#include "bitmap/color.hh"
#include "geo/int-point.hh"

namespace test::boundary_fill_source{

// Constants for use with boundary-fill-source.png
const char* fileName = "boundary-fill-source.png";
const char* fileNameKey = "boundary-fill-key.png";
constexpr faint::Color borderColor(0, 0, 0);
constexpr faint::Color keyFillColor(255, 255, 0);
constexpr faint::IntPoint fillOrigin(185, 185);

} // namespace
