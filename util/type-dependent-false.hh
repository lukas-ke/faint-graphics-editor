// -*- coding: us-ascii-unix -*-
// Copyright 2020 Lukas Kemmer
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

#ifndef FAINT_TYPE_DEPENDENT_FALSE_HH
#define FAINT_TYPE_DEPENDENT_FALSE_HH
#include <type_traits>

namespace faint{

// Helper to make static_assert:s dependent on T.
template<class T>
struct TypeDependentFalse : std::false_type {};

// Variable variant of TypeDependentFalse
template<class T>
inline constexpr bool TypeDependentFalse_v = TypeDependentFalse<T>::value;

} // namespace

#endif
