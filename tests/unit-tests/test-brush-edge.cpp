// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/text-bitmap.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/brush.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "rendering/render-brush.hh"

using namespace faint;

void test_brush_edge(){
  {
    // Rectangle brush
    auto edge = brush_edge(rect_brush(2));
    ASSERT_EQUAL(edge.size(), 4); // Rectangle brush has 4 sides

    EQUAL(edge[0], IntLineSegment({0,0}, {2,0})); // Top
    EQUAL(edge[1], IntLineSegment({0,2}, {2,2})); // Bottom
    EQUAL(edge[2], IntLineSegment({0,0}, {0,2})); // Left
    EQUAL(edge[3], IntLineSegment({2,0}, {2,2})); // Right
  }

  {
    auto edge = brush_edge(create_brush(IntSize(4,4),
        // A circular brush
        " XX "
        "XXXX"
        "XXXX"
        " XX ",
        // Using 1 as alpha-value to ensure that low alpha values > 0
        // are considered "inside".
        {{' ', 0}, {'X', 1}}));
    ASSERT_EQUAL(edge.size(), 12);

    // Top edge
    EQUAL(edge[0], IntLineSegment({1,0},{3,0}));

    // Steps from above
    EQUAL(edge[1], IntLineSegment({0,1},{1,1}));
    EQUAL(edge[2], IntLineSegment({3,1},{4,1}));

    // Steps from below
    EQUAL(edge[3], IntLineSegment({0,3},{1,3}));
    EQUAL(edge[4], IntLineSegment({3,3},{4,3}));

    // Bottom edge
    EQUAL(edge[5], IntLineSegment({1,4},{3,4}));

    // Leftmost edge
    EQUAL(edge[6], IntLineSegment({0,1},{0,3}));

    // Steps from the left
    EQUAL(edge[7], IntLineSegment({1,0},{1,1}));
    EQUAL(edge[8], IntLineSegment({1,3},{1,4}));

    // Steps from the right
    EQUAL(edge[9], IntLineSegment({3,0},{3,1}));
    EQUAL(edge[10], IntLineSegment({3,3},{3,4}));

    // Rightmost edge
    EQUAL(edge[11], IntLineSegment({4,1},{4,3}));
  }
}
