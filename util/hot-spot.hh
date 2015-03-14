#ifndef FAINT_HOT_SPOT_HH
#define FAINT_HOT_SPOT_HH
#include "geo/int-point.hh"
#include "util/distinct.hh"

namespace faint{

class category_cursor_hotspot;
using HotSpot = Subtype<IntPoint, category_cursor_hotspot, 0>;

} // namespace

#endif
