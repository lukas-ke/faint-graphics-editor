// -*- coding: us-ascii-unix -*-
#include <memory>
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "tests/test-util/print-optional.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objline.hh"
#include "text/text-expression-context.hh"
#include "util/default-settings.hh"
#include "util/object-util.hh"

static std::unique_ptr<faint::ExpressionContext> dummy_expression_context(){
  using namespace faint;
  class DummyExpressionContext : public ExpressionContext{
    Optional<Calibration> GetCalibration() const override{
      return {};
    }
    const Object* GetObject(const utf8_string&) const override{
      return nullptr;
    }
  };
  return std::make_unique<DummyExpressionContext>();
}

void test_clone_as_path(){
  using namespace faint;
  using obj_ptr = std::unique_ptr<Object>;
  Points pts({PathPt::MoveTo({10,10}),
    PathPt::LineTo({15,12}),
    PathPt::LineTo({20, 23})});

  // Create a line
  auto line = obj_ptr(create_line_object(pts, default_line_settings()));
  line->SetName({"Line 1"});
  EQUAL(line->GetMovablePoints().size(), 3);
  EQUAL(line->GetName(), utf8_string{"Line 1"});

  // Clone it as a path
  auto ctx = dummy_expression_context();
  auto pathObject1 = obj_ptr(clone_as_path(line.get(), *ctx));

  // Verify that the name and points are unchanged in the clone
  EQUAL(pathObject1->GetMovablePoints().size(), 3);
  EQUAL(pathObject1->GetName(), utf8_string("Line 1"));
  auto path1 = pathObject1->GetPath(*ctx);
  VERIFY(path1[0].type == PathPt::Type::MoveTo);

  // Clone the path to a new path, and verify that it again is unchanged.
  auto pathObject2 = obj_ptr(clone_as_path(pathObject1.get(), *ctx));
  EQUAL(pathObject2->GetMovablePoints().size(), 3);
  EQUAL(pathObject2->GetName(), utf8_string("Line 1"));
  auto path2 = pathObject2->GetPath(*ctx);
  VERIFY(path2[0].type == PathPt::Type::MoveTo);
}
