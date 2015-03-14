// -*- coding: us-ascii-unix -*-
#include <vector>
#include "test-sys/test.hh"
#include "util/index-iter.hh"

void test_index(){
  using namespace faint;

  {
    // Construction

    EQUAL(Index().Get(), 0);
    EQUAL(Index(5).Get(), 5);
  }

  {
    // Operators

    VERIFY(Index().Get() == 0);
    VERIFY(Index(5).Get() == 5);

    VERIFY(Index(2) == Index(2));
    VERIFY(!(Index(2) == Index(3)));

    VERIFY(Index(2) == 2);
    VERIFY(!(Index(2) == 3));

    VERIFY(Index(2) != Index(3));

    VERIFY(Index(2) != 3);

    VERIFY(Index(2) < Index(3));
    VERIFY(!(Index(3) < Index(3)));

    VERIFY(Index(3) <= Index(3));
    VERIFY(Index(2) <= Index(3));
    VERIFY(!(Index(3) <= Index(2)));
  }

  using index_vec = std::vector<Index>;

  {
    // up_to generator with upper-bound from literal

    index_vec v;
    for (auto i : up_to(3_idx)){
      v.push_back(i);
    }

    VERIFY(v == index_vec({0_idx, 1_idx, 2_idx}));
  }

  {
    // up_to generator with upper-bound from lambda

    auto get_end_value = [](){return 2_idx;};

    index_vec v;
    for (auto i : up_to(get_end_value())){
      v.push_back(i);
    }

    VERIFY(v == index_vec({0_idx, 1_idx}));
  }
}
