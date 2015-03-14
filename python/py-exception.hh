// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_PY_EXCEPTION_HH
#define FAINT_PY_EXCEPTION_HH
#include <vector>
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

class FaintPySyntaxError{
public:
  FaintPySyntaxError()
    : line(0), col(0)
  {}
  utf8_string file;
  int line;
  int col;
  utf8_string code;
};

class FaintPyExc{
public:
  utf8_string type;
  utf8_string message;
  std::vector<utf8_string> stackTrace;
  Optional<FaintPySyntaxError> syntaxErrorInfo;
};

} // namespace

#endif
