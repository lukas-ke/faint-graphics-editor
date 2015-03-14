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

#ifndef FAINT_PY_KEY_PRESS_HH
#define FAINT_PY_KEY_PRESS_HH
#include <vector>
#include "text/utf8-string.hh"
#include "util/key-press.hh"

namespace faint{

struct BindInfo{
  BindInfo(const KeyPress& key,
    const utf8_string& function,
    const utf8_string& docs)
    : key(key),
      function(function),
      docs(docs)
  {}
  KeyPress key;
  utf8_string function;
  utf8_string docs;
};

std::vector<BindInfo> list_binds();

void python_key_press(const KeyPress&);

} // namespace

#endif
