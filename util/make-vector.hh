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

#ifndef FAINT_MAKE_VECTOR_HH
#define FAINT_MAKE_VECTOR_HH
#include <map>
#include <vector>
#include <functional>

namespace faint{

template<typename Generator, typename Func>
auto make_vector(const Generator& gen, Func&& f){
  // Return a vector containing the results of calling f with
  // all values produced by begin(gen) to end(gen).

  using iter_t = decltype(begin(gen));
  using param_t = typename iter_t::value_type;
  using value_type =
    typename std::result_of<Func(param_t)>::type;

  std::vector<value_type> v;
  v.reserve(std::distance(begin(gen), end(gen)));

  for (const auto& value : gen){
    v.emplace_back(f(value));
  }

  return v;
}

template<typename T, typename Func>
auto make_vector(std::initializer_list<T>&& gen, Func&& f){
  // Return a vector containing the results of calling f with
  // all values produced by begin(gen) to end(gen).

  using iter_t = decltype(*begin(gen));
  using value_type =
    typename std::result_of<Func(iter_t)>::type;

  std::vector<value_type> v;
  v.reserve(std::distance(begin(gen), end(gen)));

  for (const auto& value : gen){
    v.emplace_back(f(value));
  }

  return v;
}

template<typename Generator, typename Func>
auto make_map(const Generator& gen, Func&& f){
  using iter_t = decltype(begin(gen));
  using param_t = typename iter_t::value_type;
  using pair_type =
    typename std::result_of<Func(param_t)>::type;
  using key_type = decltype(pair_type::first);
  using value_type = decltype(pair_type::second);
  std::map<key_type, value_type> m;

  for (const auto& value : gen){
    auto p = f(value);
    m[p.first] = p.second;
  }
  return m;
}

template<class T1, class T2>
T2 convert_f(const T1& v){
  return T2(v);
}

} // namespace

#endif
