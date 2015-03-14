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

#include <cassert>
#include "formats/save-result.hh"

namespace faint{

SaveResult SaveResult::SaveSuccessful(){
  return SaveResult(true, utf8_string(""));
}

SaveResult SaveResult::SaveFailed(const utf8_string& error){
  return SaveResult(false, error);
}

SaveResult::SaveResult(bool ok, const utf8_string& error)
  : m_ok(ok),
    m_error(error)
{}

bool SaveResult::Failed() const{
  return !m_ok;
}

bool SaveResult::Successful() const{
  return m_ok;
}

utf8_string SaveResult::ErrorDescription() const{
  assert(!m_ok);
  return m_error;
}

} // namespace
