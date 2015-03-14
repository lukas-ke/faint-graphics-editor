// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include <vector>
#include "geo/point-range.hh"

void test_point_range(){
  using namespace faint;

  const std::vector<IntPoint> key = {
    {2,1}, {3,1}, {4,1},
    {2,2}, {3,2}, {4,2},
    {2,3}, {3,3}, {4,3}};

  const point_range ptRange({2,1},{4,3});

  { // Test iterator construction

    const std::vector<IntPoint> v(begin(ptRange), end(ptRange));
    VERIFY(key == v);
  }

  { // Test/illustrate loop usage
    std::vector<IntPoint> v;
    for (auto pt : ptRange){
      v.push_back(pt);
    }
    VERIFY(key == v);
  }
}
