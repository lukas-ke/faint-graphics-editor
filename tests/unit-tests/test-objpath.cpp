// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objpath.hh"
#include "util/default-settings.hh"

void test_objpath(){
  using namespace faint;

  std::vector<PathPt> pts{
    MoveTo({136, 266}),
    CubicBezier({362, 210}, {159, 130}, {289, 125}),
    Close()};

  std::unique_ptr<Object> path(create_path_object(Points(pts),
    default_path_settings()));

  VERIFY(!path->Active());
  VERIFY(path->Inactive());
  KNOWN_ERROR(path->CanRemovePoint());
  VERIFY(path->CyclicPoints());
  KNOWN_ERROR(path->Extendable());

  // Start point, curve with two control points and end point
  EQUAL(path->GetMovablePoints().size(),
    4);

  NOT(path->IsControlPoint(0)); // MoveTo
  NOT(path->IsControlPoint(1)); // CurveTo end
  VERIFY(path->IsControlPoint(2)); // CurveTo c
  VERIFY(path->IsControlPoint(3)); // CurveTo d

  // TODO: Test the UndoFunc from point insertion, it has issues.
}
