// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/radii.hh"
#include "geo/rect.hh"
#include "geo/tri.hh"

const auto eps = 0.001_eps;

static void verify_same(const faint::Point& p0, const faint::Point& p1){
  NEAR(p0.x, p1.x, eps);
  NEAR(p0.y, p1.y, eps);
}

static void verify_same(const faint::Radii& r0, const faint::Radii& r1){
  NEAR(r0.x, r1.x, eps);
  NEAR(r0.y, r1.y, eps);
}

void test_tri(){
  using namespace faint;

  {
    // <../../doc/test-tri-a-1.png>
    Tri t({0,0},{10,0},{0,8});
    NEAR(t.Width(), 10.0, eps);
    NEAR(t.Height(), 8.0, eps);
    VERIFY(rather_zero(t.GetAngle()));
    NEAR(t.Skew(), 0.0, eps);
    VERIFY(t.Contains({0,0}));
    VERIFY(t.Contains({10,0}));
    VERIFY(t.Contains({0,8}));
    FWD(verify_same(bounding_rect(t).TopLeft(), {0,0}));
    FWD(verify_same(bounding_rect(t).BottomRight(), {10,8}));

    // Positions
    FWD(verify_same(center_point(t), {5.0,4.0}));
    FWD(verify_same(mid_P0_P1(t), {5.0, 0.0}));
    FWD(verify_same(mid_P0_P2(t), {0.0, 4.0}));
    FWD(verify_same(mid_P1_P3(t), {10.0, 4.0}));

    FWD(verify_same(get_radii(t), {5.0, 4.0}));
    NEAR(area(t), 80.0, eps);

    // Skew
    NEAR(skewed(t, 10.0).Skew(), 10.0, eps);
    NEAR(area(skewed(t, 10.0)), area(t), eps);
  }

  {
    // <../../doc/test-tri-b-1.png>
    Tri t({0,0}, {-10,0}, {0,8});
    NEAR(t.Width(), 10.0, eps);

    // For some (deliberate) reason, height is negative when flipped.
    KNOWN_INEQUAL(t.Height(), 8);

    VERIFY(t.GetAngle() == pi);
    NEAR(t.Skew(), 0.0, eps);
    NEAR(area(t), 80.0, eps);
    FWD(verify_same(bounding_rect(t).TopLeft(), {-10,0}));
    FWD(verify_same(bounding_rect(t).BottomRight(), {0,8}));

    // Positions
    FWD(verify_same(center_point(t), {-5, 4}));
    FWD(verify_same(mid_P0_P1(t), {-5, 0}));
    FWD(verify_same(mid_P0_P2(t), {0.0, 4.0}));
    FWD(verify_same(mid_P1_P3(t), {-10.0, 4.0}));

    // Skew
    NEAR(skewed(t, 10.0).Skew(), 10.0, eps);
    NEAR(area(skewed(t, 10.0)), area(t), eps);
  }

  {
    // Lousy degenerate
    Tri t({0,0},{0,0},{0,0});
    NEAR(t.Width(), 0.0, eps);
    NEAR(t.Height(), 0.0, eps);
    VERIFY(rather_zero(t.GetAngle()));
    NEAR(t.Skew(), 0.0, eps);
    FWD(verify_same(mid_P0_P1(t), {0,0}));
    FWD(verify_same(mid_P0_P2(t), {0,0}));
    FWD(verify_same(mid_P1_P3(t), {0,0}));
  }

}
