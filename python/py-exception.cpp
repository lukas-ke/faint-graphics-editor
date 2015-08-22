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

#include "python/py-exception.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"

namespace faint{

utf8_string format_error_info(const FaintPyExc& info){
  utf8_string errStr = no_sep(info.stackTrace);
  if (info.syntaxErrorInfo.IsSet()){
    FaintPySyntaxError syntaxError = info.syntaxErrorInfo.Get();
    errStr += "\n";
    errStr += ("  File " + quoted(syntaxError.file) + ", line " +
      str_int(syntaxError.line) + "\n");
    errStr += ("    " + syntaxError.code);
    if (syntaxError.col > 0){
      if (syntaxError.code.str().back() != '\n'){
        errStr += "\n";
      }

      // Put a '^'-under the start of the syntax error
      errStr += ("    " +
        utf8_string(to_size_t(syntaxError.col - 1), chars::space) +
        "^\n");
    }
  }
  errStr += info.type + ": " + info.message + "\n";
  errStr += "\n";
  return errStr;
}

} // namespace
