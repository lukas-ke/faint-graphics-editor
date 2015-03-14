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

#ifndef FAINT_SCOPED_ERROR_LOG_HH
#define FAINT_SCOPED_ERROR_LOG_HH
#include "text/utf8-string.hh"

namespace faint{

class ScopedErrorLogImpl;

class ScopedErrorLog{
  // Replaces the wxWidgets error logging within a scope. This
  // prevents the wxWidgets log dialog from appearing, so that the
  // messages can be displayed in a different way. Also removes
  // timestamps.
public:
  ScopedErrorLog();

  // Restores normal logging on destruction
  ~ScopedErrorLog();

  // Returns any logged messages after construction
  utf8_string GetMessages() const;

  ScopedErrorLog(const ScopedErrorLog&) = delete;
  ScopedErrorLog& operator=(const ScopedErrorLog&) = delete;
private:
  ScopedErrorLogImpl* m_impl;
};

} // namespace

#endif
