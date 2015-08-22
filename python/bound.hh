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

#ifndef FAINT_BOUND_HH
#define FAINT_BOUND_HH
#include "python/py-func-context.hh"

namespace faint{

template<typename T>
class Bound{
  // Ties an item to a PyFuncContext, so that Faint functions can get
  // ahold of both as the "self" argument.
  //
  // - For some types (T), this is required to verify that the C++-side
  //   of the object is still valid.
  //
  // - Certain functions use the context to run Commands.
public:
  Bound(T& item, PyFuncContext& ctx)
    : item(item), ctx(ctx)
  {}

  operator T&() const {
    return item;
  }

  T& item;
  PyFuncContext& ctx;
};

template<typename T>
Bound<T> bind(T& item, PyFuncContext& ctx){
  return Bound<T>(item, ctx);
}

template<typename T>
T&& bare(T&& obj){
  return obj;
}

template<typename T>
const auto& bare(const Bound<T>& obj){
  return obj.item;
}

} // namespace

#endif
