// -*- coding: us-ascii-unix -*-
#ifndef FAINT_TEST_PRINT_OPTIONAL_HH
#define FAINT_TEST_PRINT_OPTIONAL_HH
#include <ostream>
#include "util/optional.hh"

namespace faint{

template<typename T>
std::ostream& operator<<(std::ostream& o, const Optional<T>& opt){
  opt.Visit(
    [&](const T& value){
      o << "{" << value << "}";
    },
    [&](){
      o << "{}";
    });
  return o;
}

} // namespace

#endif
