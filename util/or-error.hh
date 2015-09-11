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

#ifndef FAINT_OR_ERROR_HH
#define FAINT_OR_ERROR_HH
#include "util/either.hh"

namespace faint{

class utf8_string;

template<typename T>
using OrError = Either<T, utf8_string>;

template<typename Exception, typename T>
T or_throw(OrError<T>&& o){
  return o.Visit(
    [](const T& v){
      return v;
    },
    [](const utf8_string& error) -> T {
      throw Exception(error);
    });
}

} // namespace

#endif
