// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/color.hh"
#include "geo/arc.hh"
#include "geo/rect.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "util/default-settings.hh"
#include "util/setting-id.hh"
#include "text/formatting.hh"

void img_arc(){
  using namespace faint;
  auto arcSettings = default_ellipse_settings();
  auto ellipseSettings = default_ellipse_settings();
  ellipseSettings.Set(ts_Fg, Paint(grayscale_rgba(128)));

  for (int startAngle = 0; startAngle < 360; startAngle += 40){
    for (int stopAngle = 0; stopAngle < 360; stopAngle += 40){
      const auto tri = tri_from_rect(Rect(Point(5,5), Size(200, 100)));
      Bitmap bmp({210, 110}, color_white);
      FaintDC dc(bmp);
      dc.Ellipse(tri, ellipseSettings);
      dc.Arc(tri, AngleSpan(Angle::Deg(startAngle), Angle::Deg(stopAngle)),
        arcSettings);
      save_test_image(bmp, FileName("render-arc-" +
          str_int(startAngle, left_pad(3)) + "-" +
          str_int(stopAngle, left_pad(3)) +
          ".png"));
    }
  }
}
