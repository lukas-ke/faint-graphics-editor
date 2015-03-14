// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/canvas-geo.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "util/mouse.hh"

using namespace faint;
using namespace faint::mouse;

static CanvasGeo geo(coord scale, const IntPoint& pos, const IntSize& borderSize){
  CanvasGeo g;
  g.zoom.SetApproximate(scale);
  g.pos = pos;
  g.border = borderSize;
  return g;
}

static CanvasGeo geo(coord scale, const IntPoint& pos){
  CanvasGeo g;
  g.zoom.SetApproximate(scale);
  g.pos = pos;
  return g;
}

static CanvasGeo geo(coord scale){
  return geo(scale, IntPoint(0,0));
}

static CanvasGeo geo(){
  return CanvasGeo();
}

void test_mouse(){

  { // Test image_to_view (for IntPoint)
    EQUAL(geo().border, IntSize(20,20));
    EQUAL(image_to_view(IntPoint(10,10), geo()), IntPoint(30,30));
    EQUAL(image_to_view(IntPoint(10,10), geo(2.0)), IntPoint(40,40));
    EQUAL(image_to_view(IntPoint(10,10), geo(2.0, IntPoint(20,70))),
      IntPoint(20,-30));
    EQUAL(image_to_view(IntPoint(12,11),geo(1.0, IntPoint(10,10), IntSize(30,10))),
      IntPoint(32,11));
    EQUAL(image_to_view(IntPoint(10,20),
        geo(2.0, IntPoint(10,10), IntSize(30,10))),
      IntPoint(40,40));
  }

  { // Test image_to_view (for IntRect)
    EQUAL(image_to_view(IntRect(IntPoint(10,10), IntPoint(20,20)), geo()),
      IntRect(IntPoint(30,30), IntPoint(40,40)));

    EQUAL(image_to_view(IntRect(IntPoint(10,10), IntPoint(20,20)),
        geo(2.0, IntPoint(10,10))),
      IntRect(IntPoint(30,30), IntPoint(50,50)));

    EQUAL(image_to_view(IntRect(IntPoint(10,10), IntPoint(20,20)),
        geo(2.0, IntPoint(10,10), IntSize(15,25))),
      IntRect(IntPoint(25,35), IntPoint(45,55)));
  }

  { // Test view_to_image
    EQUAL(view_to_image(IntPoint(0,0), geo()), Point(-20, -20));
    EQUAL(view_to_image(IntPoint(40,0), geo(2.0)), Point(10, -10));

    EQUAL(view_to_image(IntPoint(40,0),
      geo(2.0, IntPoint(10,10))), Point(15, -5));

    EQUAL(view_to_image(IntPoint(40,0),
      geo(2.0, IntPoint(10,10), IntSize(20,40))), Point(15, -15));
  }
}
