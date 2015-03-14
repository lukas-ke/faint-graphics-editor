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

#ifndef FAINT_MAKE_VECTOR
#define FAINT_MAKE_VECTOR
#include <vector>
#include <functional>

namespace faint{

template<typename Generator, typename Func>
auto make_vector(const Generator& gen, Func&& f){
  // Return a vector containing the results of calling f with
  // all values produced by begin(gen) to end(gen).

  using value_type =
    typename std::result_of<Func(typename Generator::value_type)>::type;

  std::vector<value_type> v;
  v.reserve(gen.size());

  for (const auto& value : gen){
    v.emplace_back(f(value));
  }

  return v;
}

} // namespace

#endif
