// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "commands/add-point-cmd.hh"
#include "commands/command.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objpath.hh"
#include "tests/test-util/stub-command-context.hh"
#include "util/default-settings.hh"
#include "tests/test-util/print-objects.hh"

void test_objpath_undo(){
  using namespace faint;

  std::vector<PathPt> pts{
    MoveTo({136, 266}),
    CubicBezier({362, 210}, {159, 130}, {289, 125}),
    Close()};

  std::unique_ptr<Object> path(create_path_object(Points(pts),
    default_path_settings()));

  auto oldPts = path->GetMovablePoints();
  EQUAL(oldPts.size(), 4);

  {
    test::StubCommandContext ctx;
    auto cmd = add_point_command(path.get(), 1, {10.0, 10.0});
    cmd->Do(ctx);
    EQUAL(path->GetMovablePoints().size(), 7); // Bezier split in half

    cmd->Undo(ctx);
    EQUAL(path->GetMovablePoints(), oldPts);
  }
}
