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

#include <functional>
#include <random>
#include "tests/test-util/pick-random-color.hh"
#include "bitmap/color.hh"

namespace faint{ namespace test{

pick_color_f random_color_picker(int seed){
  return
    [e=std::default_random_engine(seed),
     dist = std::uniform_int_distribution<int>(0,255)]
    (const utf8_string&,
     const Color&) mutable
    {
      return option(color_from_ints(dist(e),dist(e),dist(e),255));
    };
}

}} // namespace
