// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_MATH_CONSTANTS_HH
#define FAINT_MATH_CONSTANTS_HH

namespace faint{namespace math{

constexpr double e = 2.71828182845904523536028747135266249775;
constexpr double pi = 3.141592653589793238462643383279502884;
constexpr double tau = 6.283185307179586476925286766559005768;

constexpr double radians_from_degrees(double degrees){
  return (pi / 180.0) * degrees;
}

constexpr double degrees_from_radians(double radians){
  return (180.0 / pi) * radians;
}

}} // namespace

#endif
