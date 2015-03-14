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

#include "gui/accelerator-entry.hh"
#include "util/key-press.hh"

namespace faint{

AcceleratorEntry::AcceleratorEntry()
  : func([](){})
{}

AcceleratorEntry::AcceleratorEntry(const KeyPress& key,
  const std::function<void()>& func)
  : func(func),
    keyPresses({key})
{}

AcceleratorEntry::AcceleratorEntry(const KeyPress& key1,
  const KeyPress& key2,
  const std::function<void()>& func)
  : func(func),
    keyPresses({key1, key2})
{}

} // namespace
