// -*- coding: us-ascii-unix -*-
#include "test-sys/test.hh"
#include "bitmap/color.hh"
#include "geo/range.hh"
#include "util/make-vector.hh"
#include <iterator>

namespace faint{

class range_iter_t{
public:
  using value_type = int;
  using difference_type = int;
  using pointer = int*;
  using reference = int&;
  using iterator_category = std::input_iterator_tag;

  explicit range_iter_t(int current)
    : m_current(current)
  {}

  int operator*() const{
    return m_current;
  }

  void operator++(){
    m_current++;
  }

  range_iter_t& operator++(int){
    m_current++;
    return *this;
  }

  bool operator==(const range_iter_t& rhs){
    return m_current == rhs.m_current;
  }

  bool operator!=(const range_iter_t& rhs){
    return m_current != rhs.m_current;
  }

private:
  int m_current;
};

inline auto begin(const ClosedIntRange& r){
  return range_iter_t(r.Lower());
}

inline auto end(const ClosedIntRange& r){
  return range_iter_t(r.Upper() + 1);
}

} // namespace

void test_range_iter(){
  using namespace faint;
  auto v = make_vector(make_closed_range(0,255), grayscale_rgb);
  EQUAL(v.size(), 256);
  VERIFY(v[0] == ColRGB(0,0,0));
  VERIFY(v[128] == ColRGB(128,128,128));
  VERIFY(v[255] == ColRGB(255,255,255));
}
