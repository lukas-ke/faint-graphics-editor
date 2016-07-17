// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include <memory> // unique_ptr
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/pattern.hh"

void test_pattern(){
  using namespace faint;

  const auto patternSize = IntSize(10,11);
  const auto bmp = Bitmap(patternSize, color_white);

  auto p1 = std::make_unique<Pattern>(bmp);

  // Pattern default settings
  EQUAL(p1->GetAnchor(), IntPoint(0,0));
  NOT(p1->GetObjectAligned());

  // Same pattern size as bitmap size
  EQUAL(p1->GetSize(), patternSize);

  // Identity
  VERIFY(*p1 == *p1);

  auto p2 = std::make_unique<Pattern>(bmp);
  // A pattern created with the same bitmap is NOT equal to the other pattern,
  // as this only concerns reference counting, not bitmap content
  VERIFY(*p1 != *p2);
  VERIFY(*p1 < *p2); // p2 has a higher id for reference counting

  // However, a copied pattern uses the same backing bitmap (reference counted)
  // and is equal
  auto p3 = std::make_unique<Pattern>(*p1);
  VERIFY(*p3 == *p1);

  auto p4 = std::make_unique<Pattern>(*p1);

  // Anchor is considered for equality
  p4->SetAnchor({0, 1});
  VERIFY(*p4 != *p1);
  VERIFY(*p1 < *p4);
  VERIFY(*p4 > *p1);

  // Restore p4 anchor
  p4->SetAnchor({0,0});
  VERIFY(*p4 == *p1);

  // Object aligned is considered for equality
  p4->SetObjectAligned(true);
  VERIFY(*p4 != *p1);
  VERIFY(*p1 < *p4);
  VERIFY(*p4 > *p1);

  VERIFY(offsat(*p1, {10,10}).RefId() == p1->RefId());

  auto ref_count = [](int id){
    // Retrieves the number of references to a backing bitmap with
    // this id
    auto ps = pattern_status();
    auto it = ps.find(id);
    if (it == end(ps)){
      return 0;
    }
    else{
      ASSERT(it->second != 0); // Should be removed, not reach 0
      return it->second;
    }
  };

  auto reset = [](std::unique_ptr<Pattern>& p){
    // Reset the pointer, return the id of the backing bitmap
    auto id = p->RefId();
    p.reset(nullptr);
    return id;
  };

  // p1 and p2 have separate backing bitmaps, p3 and p4 use the same
  // backing bitmap as b1.
  EQUAL(pattern_status().size(), 2);

  // p1, p3 and p4 refer to the same bitmap
  EQUAL(ref_count(p1->RefId()), 3);

  // Only p2 refers to its bitmap
  EQUAL(ref_count(p2->RefId()), 1);

  const auto p1_id = reset(p1);

  // Now only p3 and p4 refer to this bitmap
  EQUAL(ref_count(p1_id), 2);
  EQUAL(ref_count(p2->RefId()), 1); // Unaffected

  const auto p2_id = reset(p2);
  EQUAL(ref_count(p1_id), 2); // Unaffected
  EQUAL(ref_count(p2_id), 0); // No one refers to this anymore

  const auto p4_id = reset(p4);
  EQUAL(ref_count(p1_id), 1); // Only p3 still refers

  const auto p3_id = reset(p3);
  EQUAL(ref_count(p1_id), 0);

  // No pattern backing bitmaps remain
  EQUAL(pattern_status().size(), 0);
}
