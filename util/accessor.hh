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

#ifndef FAINT_ACCESSOR_HH
#define FAINT_ACCESSOR_HH
#include <functional>

namespace faint{

template<typename T>
class Accessor{
  // Class template for accessing and setting a value presumably owned
  // by someone else, through constructor specified getter and setter
  // lambdas
  //
  // This can be passed to controls, to let them access and modify the
  // value, without knowing about retrieval intricacies, graphics
  // refresh requirements and such. These details are instead handled
  // where the Accessor is instantiated.
public:
  using getter_t = std::function<T()>;
  using setter_t = std::function<void(const T&)>;
  Accessor(const getter_t& getter, const setter_t& setter)
    : Get(getter),
      Set(setter)
  {}

  getter_t Get;
  setter_t Set;
};

} // namespace

#endif
