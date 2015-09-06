// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_DELAY_HH
#define FAINT_DELAY_HH
#include <chrono>

namespace faint{

class category_frame_properties;

// Hundredths of a second is used as the base for delays (since it is what the
// gif-format uses).
using jiffies_t = std::chrono::duration<int, std::centi>;

constexpr jiffies_t operator "" _cs(unsigned long long centiSeconds){
  return jiffies_t(centiSeconds);
}

// The duration an animation should remain on a given frame.
using Delay = Distinct<jiffies_t, category_frame_properties, 0>;

} // namespace

#endif
