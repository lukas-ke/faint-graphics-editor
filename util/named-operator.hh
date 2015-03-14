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

// Note:
// The implementation of named operators is directly based on
// https://github.com/klmr/named-operator
// by klmr.

#ifndef FAINT_NAMED_OPERATOR
#define FAINT_NAMED_OPERATOR
#include <utility>

namespace faint{

namespace namedop{

template<typename F>
struct named_operator_wrapper{
  F f;
};

template<typename T, typename F>
struct named_operator_lhs {
  F f;
  T& value;
  named_operator_lhs() = delete;
  named_operator_lhs& operator=(const named_operator_lhs&) = delete;
};

template<typename T, typename F>
named_operator_lhs<T, F> operator<(T& lhs, named_operator_wrapper<F> rhs){
  return {rhs.f, lhs};
}

template<typename T, typename F>
named_operator_lhs<T const, F> operator<(T const& lhs,
  named_operator_wrapper<F> rhs)
{
  return {rhs.f, lhs};
}

template<typename T1, typename T2, typename F>
auto operator>(named_operator_lhs<T1, F> const& lhs, T2 const& rhs){
  return lhs.f(lhs.value, rhs);
}

template<typename T1, typename T2, typename F>
auto operator>=(named_operator_lhs<T1, F> const& lhs, T2 const& rhs){
  return lhs.value = lhs.f(lhs.value, rhs);
}

} // namespace

namespace op_funcs{
inline std::pair<int, int> plus_minus_f(int value, int delta){
  return {value - delta, value + delta};
}

} // namespace

template <typename F>
inline constexpr namedop::named_operator_wrapper<F> make_named_operator(F f) {
  return {f};
}

// Returns a pair of {lhs - rhs, lhs + rhs}
static const auto plus_minus = make_named_operator(op_funcs::plus_minus_f);

} // namespace

#endif
