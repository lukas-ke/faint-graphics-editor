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

#ifndef FAINT_ACCELERATOR_ENTRY_HH
#define FAINT_ACCELERATOR_ENTRY_HH
#include <functional>
#include <vector>

namespace faint{

class KeyPress;

using accel_fn = std::function<void()>;

class AcceleratorEntry{
  // Maps one or more keypresses to a lambda.

public:
  // Default entry, no accelerator, useful for conditional
  // accelerators (see cond-function)
  AcceleratorEntry();

  AcceleratorEntry(const KeyPress&, const accel_fn&);
  AcceleratorEntry(const KeyPress& key1,
    const KeyPress& key2,
    const accel_fn&);

  accel_fn func;
  std::vector<KeyPress> keyPresses;
};

template<typename ...Args>
AcceleratorEntry cond(bool condition, Args&& ...args){
  // Function for adding an accelerator within set_accelerators if a
  // condition is true.

  return condition ?
    AcceleratorEntry(std::forward<Args>(args)...) :
    AcceleratorEntry();
}

} // namespace

#endif
