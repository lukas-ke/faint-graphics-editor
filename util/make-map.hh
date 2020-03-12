// -*- coding: us-ascii-unix -*-
// Copyright 2016 Lukas Kemmer
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

#ifndef FAINT_MAKE_MAP_HH
#define FAINT_MAKE_MAP_HH
#include <map>
#include <functional>

namespace faint{

template<typename Generator, typename Func>
auto make_map(const Generator& gen, Func&& f){
  // Returns a map created by calling f with all values produced by gen,
  // and using the pairs returned by f as (key, value)

  using iter_t = decltype(begin(gen));
  using param_t = typename iter_t::value_type;
  using pair_type = std::invoke_result_t<Func, param_t>;
  using key_type = decltype(pair_type::first);
  using value_type = decltype(pair_type::second);
  std::map<key_type, value_type> m;

  for (const auto& v : gen){
    const auto& [key, value] = f(v);
    m[key] = value;
  }
  return m;
}

} // namespace

#endif
