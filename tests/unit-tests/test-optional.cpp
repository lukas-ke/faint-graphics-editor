// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "util/optional.hh"
#include "geo/int-rect.hh"

namespace {

class FailIfCopied{
public:
  FailIfCopied(int value)
    : value(value),
      dummy(false)
  {}

  FailIfCopied(const FailIfCopied&)
    : value(-1),
      dummy(false)
  {
    VERIFY(dummy);
    // FAIL();
  }
  int value;
  bool dummy;
};

class IntHolder{
public:
  explicit IntHolder(int i)
    : value(i)
  {}

  faint::Optional<int&> Get(bool c){
    if (c){
      return faint::Optional<int&>(value);
    }
    return faint::Optional<int&>();
  }

  int value;
};

} // namespace

void test_optional(){
  using namespace faint;

  // Helper constants for testing the Optional
  const IntSize altSize = IntSize(5,5);
  const IntSize bmpSize = IntSize(10,10);
  const Bitmap alt(altSize);
  const Bitmap bmp(bmpSize);
  VERIFY(alt.GetSize() != bmp.GetSize());

  // Uninitialized optional (not set).
  Optional<Bitmap> optional;
  VERIFY(optional.NotSet());
  VERIFY(!optional.IsSet());
  VERIFY(!optional);

  EQUAL(optional.Or(alt).GetSize(), altSize);
  optional.Visit(FAIL_IF_CALLED());
  optional.Visit(FAIL_IF_CALLED(), FAIL_UNLESS_CALLED());

  optional.Set(bmp);
  VERIFY(!optional.NotSet());
  VERIFY(optional.IsSet());
  VERIFY(optional);
  EQUAL(optional.Or(alt).GetSize(), bmpSize);
  optional.Visit(FAIL_UNLESS_CALLED());
  optional.Visit(FAIL_UNLESS_CALLED(), FAIL_IF_CALLED());
  IntSize sz = optional.VisitSimple(
    [](const Bitmap& bmp){
      return bmp.GetSize();
    },
    alt.GetSize());
  EQUAL(sz, bmpSize);
  EQUAL(optional.Get().GetSize(), bmpSize);

  // Take the object (clearing the optional)
  Bitmap bmp2 = optional.Take();
  EQUAL(bmp2.GetSize(), bmp.GetSize());
  VERIFY(optional.NotSet());

  // Initializing construction
  Optional<Bitmap> optional2(bmp2);
  VERIFY(optional2.IsSet());
  EQUAL(optional2.Get().GetSize(), bmp.GetSize());
  optional2.Clear();
  VERIFY(optional2.NotSet());

  // Reference
  int i = 7;
  Optional<int&> oi(i);
  oi.Get() = 8;
  EQUAL(i, 8);

  // Non-reference
  int j = 7;
  Optional<int> oj(j);
  oj.Get() = 8;
  EQUAL(j, 7);

  // Ensure that copying a reference-Optional
  // does not copy the contained value.
  FailIfCopied f(10);
  Optional<FailIfCopied&> o(f);
  o.Get().value++;
  EQUAL(o.Get().value, 11);

  Optional<FailIfCopied&> o2(o);
  VERIFY(o2.IsSet());
  o2.Get().value++;
  EQUAL(o.Get().value, 12);

  IntHolder h(12);
  EQUAL(h.value, 12);
  h.Get(true).Get()++;
  EQUAL(h.value, 13);
  EQUAL(h.Get(true).Get(), 13);
}
