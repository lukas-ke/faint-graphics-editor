// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/handle-func.hh"
#include "geo/object-handle.hh"
#include "geo/rect.hh"
#include "geo/size.hh"

void test_object_handle(){
  using namespace faint;
  using H = Handle;
  VERIFY(corner_handle(H::P0));
  VERIFY(corner_handle(H::P1));
  VERIFY(corner_handle(H::P2));
  VERIFY(corner_handle(H::P3));
  VERIFY(corner_handle(H::P3));
  VERIFY(!corner_handle(H::P0P2));
  VERIFY(!corner_handle(H::P1P3));
  VERIFY(!corner_handle(H::P0P1));
  VERIFY(!corner_handle(H::P2P3));

  Tri tri(Point(0,0), Point(12,0), Point(1,13));
  EQUAL(get<H::P0>(tri), tri.P0());
  EQUAL(get<H::P1>(tri), tri.P1());
  EQUAL(get<H::P2>(tri), tri.P2());
  EQUAL(get<H::P3>(tri), tri.P3());
  EQUAL(get<H::P0P2>(tri), LineSegment(tri.P0(), tri.P2()));
  EQUAL(get<H::P1P3>(tri), LineSegment(tri.P1(), tri.P3()));
  EQUAL(get<H::P0P1>(tri), LineSegment(tri.P0(), tri.P1()));
  EQUAL(get<H::P2P3>(tri), LineSegment(tri.P2(), tri.P3()));

  Rect r(Point(0,0), Size(10,10));
  EQUAL(get<H::P0>(r), r.TopLeft());
  EQUAL(get<H::P1>(r), r.TopRight());
  EQUAL(get<H::P2>(r), r.BottomLeft());
  EQUAL(get<H::P3>(r), r.BottomRight());
  EQUAL(get<H::P0P2>(r), LineSegment(r.TopLeft(), r.BottomLeft()));
  EQUAL(get<H::P1P3>(r), LineSegment(r.TopRight(), r.BottomRight()));
  EQUAL(get<H::P0P1>(r), LineSegment(r.TopLeft(), r.TopRight()));
  EQUAL(get<H::P2P3>(r), LineSegment(r.BottomLeft(), r.BottomRight()));
}
