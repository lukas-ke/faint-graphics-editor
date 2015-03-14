// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "util/zoom-level.hh"

using namespace faint;

void test_zoom_level(){
  ZoomLevel zoom;
  VERIFY(zoom.At100());
  VERIFY(!zoom.AtMax());
  VERIFY(!zoom.AtMin());
  EQUAL(zoom.GetScaleFactor(), 1.0);

  VERIFY(zoom.Next());
  VERIFY(!zoom.At100());
  EQUAL(zoom.GetPercentage(), 200);
  EQUAL(zoom.GetScaleFactor(), 2.0);
  VERIFY(zoom.Change(ZoomLevel::NEXT));

  zoom.SetApproximate(2.3);
  EQUAL(zoom.GetPercentage(), 200);

  zoom.Change(ZoomLevel::DEFAULT);
  EQUAL(zoom.GetPercentage(), 100);
  VERIFY(zoom.At100());
}
