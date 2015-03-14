// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef FAINT_GENERATOR_ADAPTER_HH
#define FAINT_GENERATOR_ADAPTER_HH
#include <algorithm>
#include <numeric>
#include "util/optional.hh"

namespace faint{

template<class Generator, class UnaryPredicate>
bool any_of(Generator gen, UnaryPredicate p){
  return std::any_of(begin(gen), end(gen), p);
}

template<class Generator, class UnaryPredicate>
bool all_of(Generator gen, UnaryPredicate p){
  return std::all_of(begin(gen), end(gen), p);
}

template<class Generator, class UnaryPredicate>
auto find_if(Generator gen, UnaryPredicate p){
  auto it = std::find_if(begin(gen), end(gen), p);
  if (it != end(gen)){
    return Optional<decltype(*it)>(*it);
  }
  return Optional<decltype(*it)>();
}

template<class Container, class T, class BinaryOperation>
auto accumulate(T initial, const Container& c, BinaryOperation op){
  return std::accumulate(begin(c), end(c), initial, op);
}

template<class Container>
Container sorted(const Container& c){
  Container c2(c);
  sort(begin(c2), end(c2));
  return c2;
}

} // namespace

#endif
