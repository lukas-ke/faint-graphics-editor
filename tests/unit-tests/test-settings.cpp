// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "tests/test-util/print-objects.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "util/bound-setting.hh"
#include "util/settings.hh"

enum class Enum1{
  E1_Value1,
  E1_Value2
};

int to_int(Enum1 e){
  return static_cast<int>(e);
}

enum class Enum2{
  E2_Value1,
  E2_Value2
};

int to_int(Enum2 e){
  return static_cast<int>(e);
}

void test_settings(){
  using namespace faint;
  BoolSetting bool1;
  BoolSetting bool2;
  IntSetting int1;
  IntSetting int2;
  ColorSetting color1;
  ColorSetting color2;
  StringSetting str1;
  StringSetting str2;
  EnumSetting<Enum1> enum1;
  EnumSetting<Enum2> enum2;
  const Paint red(Color(255,0,0));
  const Paint green(Color(0,255,0));
  BoolSetting extraBool;
  IntSetting extraInt;
  Settings s;


  // BoolSetting: Basics
  VERIFY(s.Empty());
  VERIFY(s.Lacks(bool1));
  VERIFY(!s.Has(bool1));
  VERIFY(s.GetDefault(bool1, true));
  VERIFY(!s.GetDefault(bool1, false));

  s.Set(bool1, true);
  VERIFY(s.Has(bool1));
  VERIFY(!s.Lacks(bool1));
  VERIFY(s.Get(bool1));

  // BoolSetting: Prefer actual value to default
  VERIFY(s.GetDefault(bool1, false));

  VERIFY(s.Lacks(bool2));
  s.Set(bool2, true);
  VERIFY(s.Has(bool2));
  VERIFY(!s.Lacks(bool2));
  VERIFY(s.Get(bool2));
  VERIFY(s.Get(bool1));
  s.Set(bool2, false);
  VERIFY(!s.Get(bool2));
  VERIFY(s.Get(bool1));

  // EnumSetting: Basics
  VERIFY(!s.Has(enum1));
  VERIFY(s.Lacks(enum1));
  s.Set(enum1, Enum1::E1_Value1);
  VERIFY(s.Has(enum1));
  VERIFY(!s.Has(enum2));
  VERIFY(s.Get(enum1) == Enum1::E1_Value1);

  // EnumSetting: Value change
  s.Set(enum1, Enum1::E1_Value2);
  VERIFY(s.Get(enum1) == Enum1::E1_Value2);

  // EnumSetting: Update existing value
  bool updated = s.Update(BoundSetting(enum1, Enum1::E1_Value1));
  VERIFY(updated);
  VERIFY(s.Get(enum1) == Enum1::E1_Value1);

  // EnumSetting: Update does not insert missing elements
  updated = s.Update(BoundSetting(enum2, Enum2::E2_Value1));
  VERIFY(!updated);
  VERIFY(s.Lacks(enum2));

  // EnumSetting: Inserting new value
  s.Set(enum2, Enum2::E2_Value1);
  VERIFY(s.Get(enum2) == Enum2::E2_Value1);

  // IntSetting
  VERIFY(!s.Has(int1));
  VERIFY(s.Lacks(int1));
  s.Set(int1, 1);
  s.Set(int2, 2);
  EQUAL(s.Get(int1), 1);
  EQUAL(s.Get(int2), 2);


  // StringSetting
  s.Set(str1, "First");
  s.Set(str2, "Second");
  EQUAL(s.Get(str1), "First");
  EQUAL(s.Get(str2), "Second");

  // ColorSetting
  VERIFY(s.Lacks(color1));
  VERIFY(!s.Has(color1));

  s.Set(color1, red);
  VERIFY(s.Has(color1));
  EQUAL(s.Get(color1), red);
  VERIFY(!s.Has(color2));

  // ColorSetting: No change update
  updated = s.Update(BoundSetting(color1, red));
  VERIFY(!updated);
  EQUAL(s.Get(color1), red);

  // ColorSetting: Modifying update
  updated = s.Update(BoundSetting(color1, green));
  VERIFY(updated);
  EQUAL(s.Get(color1), green);
  s.Set(color2, red);
  EQUAL(s.Get(color1), green);
  EQUAL(s.Get(color2), red);

  // Update does not insert missing keys
  Settings s2;
  s2.Set(extraBool, true);
  s2.Set(extraInt, 10);

  VERIFY(!s.Update(s2));
  VERIFY(!s.Has(extraBool));
  VERIFY(!s.Has(extraInt));
  VERIFY(s2.Get(extraBool));
  EQUAL(s2.Get(extraInt), 10);

  // UpdateAll inserts missing keys
  s.UpdateAll(s2);
  VERIFY(s.Get(extraBool));
  EQUAL(s.Get(extraInt), 10);

  // Operations: without
  auto s3 = without(s, bool1);
  VERIFY(s.Has(bool1));
  VERIFY(s3.Lacks(bool1));
  VERIFY(s3.Has(bool2));
  VERIFY(!s3.Get(bool2));

  // Two-argment without,
  s3 = without(s, bool1, bool2);
  VERIFY(s3.Lacks(bool1));
  VERIFY(s3.Lacks(bool2));

  // without allows missing values
  s3 = without(s3, bool1, bool2);
  VERIFY(s3.Has(str1));
  s3.Clear();
  VERIFY(!s3.Has(str1));
  VERIFY(s3.Empty());

  updated = s.Update(s);
  VERIFY(!updated);
  updated = s3.Update(s);
  VERIFY(!updated);
  s3.Set(str1, "Different");
  updated = s3.Update(s);
  VERIFY(updated);
  EQUAL(s3.Get(str1), "First");
  VERIFY(!s3.Has(str2));
  s3 = s;
  EQUAL(s3.Get(str2), "Second");
}
