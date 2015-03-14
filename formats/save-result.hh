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

#ifndef FAINT_SAVE_RESULT_HH
#define FAINT_SAVE_RESULT_HH
#include "text/utf8-string.hh"

namespace faint{

class SaveResult{
public:
  static SaveResult SaveSuccessful();
  static SaveResult SaveFailed(const utf8_string& error);
  bool Failed() const;
  bool Successful() const;
  utf8_string ErrorDescription() const;
private:
  SaveResult(bool, const utf8_string&);
  bool m_ok;
  utf8_string m_error;
};

} // namespace

#endif
