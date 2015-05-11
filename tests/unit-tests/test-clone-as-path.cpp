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

namespace {

std::unique_ptr<faint::ExpressionContext> dummy_expression_context(){
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

}

void test_clone_as_path(){
  using namespace faint;
  Points pts({PathPt::MoveTo({10,10}),
    PathPt::LineTo({15,15}),
    PathPt::LineTo({20, 20})});

  auto line = std::unique_ptr<Object>(create_line_object(pts,
    default_line_settings()));
  line->SetName({"Line 1"});
  EQUAL(line->GetMovablePoints().size(), 3);
  EQUAL(line->GetName(), utf8_string{"Line 1"});

  auto ctx = dummy_expression_context();
  auto path = std::unique_ptr<Object>(clone_as_path(line.get(), *ctx));

  EQUAL(path->GetMovablePoints().size(), 3);
  EQUAL(path->GetName(), utf8_string("Line 1"));
}
