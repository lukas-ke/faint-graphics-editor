// -*- coding: us-ascii-unix -*-
#include <memory>
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/measure.hh"
#include "geo/point.hh"
#include "geo/points.hh"
#include "geo/rect.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objcomposite.hh"
#include "objects/objline.hh"
#include "objects/objpolygon.hh"
#include "objects/objrectangle.hh"
#include "util/default-settings.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"

void test_object_util(){
  using namespace faint;

  std::unique_ptr<Object> rect(create_rectangle_object(Tri(Point(0,0),
    Point(10,0), Point(0,5)), default_rectangle_settings()));

  Points pts;
  pts.Append(Point(0,10));
  pts.Append(Point(10,0));
  pts.Append(Point(20,10));
  std::unique_ptr<Object> polygon(create_polygon_object(pts,
    default_polygon_settings()));

  {
    // Test "area"
    EQUAL(rounded(object_area(rect.get())), 50);

    // Triangle polygon area should be 10 * 20 / 2
    KNOWN_ERROR(rounded(object_area(polygon.get())) == 100);
  }

  {
    // Test "bounding_rect", "get_collective_name"

    Rect r = bounding_rect(LIST(rect.get(), polygon.get()));

    EQUAL(rounded(r.TopLeft()), IntPoint(0,0));
    EQUAL(rounded(r.BottomRight()), IntPoint(20,10));
    // Test "get_collective_name"
    EQUAL(get_collective_type({rect.get(), polygon.get()}), "Objects");

    EQUAL(get_collective_type(LIST(rect.get())), "Rectangle");

    EQUAL(get_collective_type(LIST(polygon.get())), "Polygon");

    std::unique_ptr<Object> r2(create_rectangle_object(Tri(Point(0,0),
      Point(10,0), Point(0,10)),
      default_rectangle_settings()));
    EQUAL(get_collective_type(LIST(rect.get(), r2.get())), "Rectangles");
  }

  {
    // Test "toggle_edit_points"
    VERIFY(supports_point_editing(polygon.get()));
    NOT(polygon->GetSettings().Get(ts_EditPoints));
    VERIFY(resize_handles_enabled(polygon.get()));

    VERIFY(toggle_edit_points(polygon.get()));
    VERIFY(polygon->GetSettings().Get(ts_EditPoints));
    VERIFY(point_edit_enabled(polygon.get()));
    NOT(resize_handles_enabled(polygon.get()));
    VERIFY(supports_point_editing(polygon.get()));

    VERIFY(toggle_edit_points(polygon.get()));
    NOT(polygon->GetSettings().Get(ts_EditPoints));
    NOT(point_edit_enabled(polygon.get()));
    VERIFY(resize_handles_enabled(polygon.get()));

    VERIFY(resize_handles_enabled(rect.get()));
    VERIFY(rect->GetSettings().Has(ts_EditPoints));
    VERIFY(toggle_edit_points(rect.get()));
    VERIFY(rect->GetSettings().Has(ts_EditPoints));
    VERIFY(supports_point_editing(rect.get()));

    VERIFY(supports_object_aligned_resize(polygon.get()));
    VERIFY(supports_object_aligned_resize(rect.get()));
  }

  {
    // Test Polygon "prev_point", "next_point"
    VERIFY(polygon->CyclicPoints());
    EQUAL(rounded(prev_point(polygon.get(), 0)), IntPoint(20,10));
    EQUAL(rounded(prev_point(polygon.get(), 1)), IntPoint(0,10));
    EQUAL(rounded(prev_point(polygon.get(), 2)), IntPoint(10,0));

    EQUAL(rounded(next_point(polygon.get(), 0)), IntPoint(10,0));
    EQUAL(rounded(next_point(polygon.get(), 1)), IntPoint(20,10));
    EQUAL(rounded(next_point(polygon.get(), 2)), IntPoint(0,10));
  }

  {
    // Test PolyLine "prev_point", "next_point"
    std::unique_ptr<Object> line(create_line_object(pts,
      default_line_settings()));
    VERIFY(!line->CyclicPoints());

    // Weird: Prev at start becomes next for lines (non-cyclic).
    // Probably used for constraining, but rather unexpected.
    EQUAL(rounded(prev_point(line.get(), 0)), IntPoint(10,0));
    EQUAL(rounded(prev_point(line.get(), 1)), IntPoint(0,10));
    EQUAL(rounded(prev_point(line.get(), 2)), IntPoint(10,0));

    EQUAL(rounded(next_point(line.get(), 0)), IntPoint(10,0));
    EQUAL(rounded(next_point(line.get(), 1)), IntPoint(20,10));

    // Weird: Next at end becomes prev for lines (non-cyclic).
    EQUAL(rounded(next_point(line.get(), 2)), IntPoint(10,0));
  }

  {
    // Test "get_by_name"
    std::unique_ptr<Object> rect1(create_rectangle_object(Tri(Point(0,0),
       Point(10,0), Point(0,5)), default_rectangle_settings()));
    const utf8_string nameRect1("rect1");

    // Single rectangle
    VERIFY(get_by_name(rect1.get(), nameRect1) == nullptr);
    rect1->SetName(option(nameRect1));
    VERIFY(get_by_name(rect1.get(), nameRect1) == rect1.get());

    objects_t objs;
    VERIFY(get_by_name(objs, nameRect1) == nullptr);
    VERIFY(get_by_name(objs, "") == nullptr);

    objs.emplace_back(rect1.get());
    VERIFY(get_by_name(objs, "") == nullptr);
    VERIFY(get_by_name(objs, nameRect1) == rect1.get());

    std::unique_ptr<Object> rect2(create_rectangle_object(Tri(Point(0,0),
       Point(10,0), Point(0,5)), default_rectangle_settings()));
    const utf8_string nameRect2("rect2");
    rect2->SetName(option(nameRect2));
    objs.emplace_back(rect2.get());

    VERIFY(get_by_name(objs, nameRect1) == rect1.get());
    VERIFY(get_by_name(objs, nameRect2) == rect2.get());

    // Group
    std::unique_ptr<Object> rect3(create_rectangle_object(Tri(Point(0,0),
       Point(10,0), Point(0,5)), default_rectangle_settings()));
    const utf8_string nameRect3("rect3");
    rect3->SetName(option(nameRect3));

    std::unique_ptr<Object> rect4(create_rectangle_object(Tri(Point(0,0),
       Point(10,0), Point(0,5)), default_rectangle_settings()));
    const utf8_string nameRect4("rect4");
    rect4->SetName(option(nameRect4));

    std::unique_ptr<Object> group1(create_composite_object(
      {rect3.get(), rect4.get()}, Ownership::LOANER));
    const utf8_string nameGroup1("group1");
    group1->SetName(option(nameGroup1));

    objs.emplace_back(group1.get());

    VERIFY(get_by_name(objs, nameGroup1) == group1.get());
    VERIFY(get_by_name(objs, nameRect1) == rect1.get());
    VERIFY(get_by_name(objs, nameRect2) == rect2.get());
    VERIFY(get_by_name(objs, nameRect3) == rect3.get());
    VERIFY(get_by_name(objs, nameRect4) == rect4.get());
    VERIFY(get_by_name(objs, "Hello") == nullptr);
  }
}
