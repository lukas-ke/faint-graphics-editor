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

#ifndef FAINT_TYPE_UTIL_HH
#define FAINT_TYPE_UTIL_HH
#include <memory>

namespace faint{

template<typename T2, typename T1>
std::unique_ptr<T2> unique_ptr_cast(std::unique_ptr<T1> src){
  assert(src != nullptr);
  auto* p = dynamic_cast<T2*>(src.get());
  assert(p != nullptr);
  src.release();
  return std::unique_ptr<T2>(p);
}

template<class Expected, class T>
bool is_type(const T& o){
  return dynamic_cast<const Expected*>(&o) != nullptr;
}

template<typename Dst, typename Src, typename Func>
void if_type(Src&& vSrc, Func&& f){
  auto* vDst = dynamic_cast<Dst*>(&vSrc);
  if (vDst != nullptr){
    f(*vDst);
  }
}


template<typename Dst, typename Src, typename Func, typename Func2>
auto if_type(Src&& vSrc, Func&& f, Func2&& otherwise){
  auto* vDst = dynamic_cast<Dst*>(&vSrc);
  if (vDst != nullptr){
    return f(*vDst);
  }
  else{
    return otherwise();
  }
}

inline bool false_f(){
  return false;
}

template<class T1, class T2>
T2 convert_f(const T1& v){
  return T2(v);
}

// Helper to make static_assert:s dependent on T.
template<class T>
struct TypeDependentFalse_s : std::false_type {};

// Variable variant of TypeDependentFalse
template<class T>
inline constexpr bool TypeDependentFalse = TypeDependentFalse_s<T>::value;

} // namespace

#endif
