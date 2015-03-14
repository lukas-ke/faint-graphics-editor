// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/measure.hh"
#include "geo/pathpt.hh"

void test_bezier_distance(){
  using namespace faint;
  static const int subDivisions = 10;

  const coord lineLength = distance(Point(0,0), Point(10,10));
  NEAR(lineLength, 14.1421, 0.0001_eps);

  // Compare straight line with straight-bezier (coincident control
  // points).
  const coord straightBezierLength = distance(Point(0,0),
    CubicBezier({10,10}, {5,5}, {5,5}), subDivisions);
  NEAR(straightBezierLength, lineLength, 0.0001_eps);

  // Verify that a curved bezier is longer than a straight bezier
  // between the same points.
  const coord curvedBezierLength = distance(Point(0,0),
    CubicBezier({10,10}, {10,0}, {10,0}), subDivisions);
  VERIFY(curvedBezierLength - straightBezierLength > 1);

  // Verify that the curved bezier is shorter than the
  // Manhattan-distance.
  VERIFY(curvedBezierLength <
    distance(Point(0,0), Point(10,0)) +
    distance(Point(10, 0), Point(10,10)));
}
