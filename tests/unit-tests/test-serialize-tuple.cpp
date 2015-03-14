// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include <cstdint>
#include <iomanip>
#include "tests/test-util/print-objects.hh"
#include "util/deserialize-tuple.hh"
#include "util/serialize-tuple.hh"

void test_serialize_tuple(){
  using namespace faint;

  static_assert(sizeof_entries<uint16_t, uint32_t, uint16_t, uint16_t, uint32_t>::value == 14, "Incorrect byte-size");
  std::tuple<uint16_t, uint32_t, uint16_t, uint16_t, uint32_t, uint8_t> t(
    0x424d,
    0x12345678,
    0x424d,
    0x424d,
    0xfefefefe,
    0xab);
  auto a = serialize_tuple(t);
  ABORT_IF(a.size() != 15);

  EQUAL_HEX(a[0], 0x4d);
  EQUAL_HEX(a[1], 0x42);

  EQUAL_HEX(a[2], 0x78);
  EQUAL_HEX(a[3], 0x56);
  EQUAL_HEX(a[4], 0x34);
  EQUAL_HEX(a[5], 0x12);

  EQUAL_HEX(a[6], 0x4d);
  EQUAL_HEX(a[7], 0x42);

  EQUAL_HEX(a[8], 0x4d);
  EQUAL_HEX(a[9], 0x42);

  EQUAL_HEX(a[10], 0xfe);
  EQUAL_HEX(a[11], 0xfe);
  EQUAL_HEX(a[12], 0xfe);
  EQUAL_HEX(a[13], 0xfe);
  EQUAL_HEX(a[14], 0xab);

  auto t2 = deserialize_tuple2<decltype(t)>(a);
  VERIFY(t == t2);
  EQUAL(std::get<0>(t2), 0x424du);
  EQUAL(std::get<1>(t2), 0x12345678u);
  EQUAL(std::get<2>(t2), 0x424du);
  EQUAL(std::get<3>(t2), 0x424du);
  EQUAL(std::get<4>(t2), 0xfefefefeu);
  EQUAL(std::get<5>(t2), 0xabu);
}
