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

#include <sstream>
#include "wx/log.h"
#include "util-wx/scoped-error-log.hh"

namespace faint{

class ScopedErrorLogImpl{
public:
  ScopedErrorLogImpl()
    : m_stream(""),
      m_logTarget(&m_stream),
      m_prevTarget(wxLog::SetActiveTarget(&m_logTarget)),
      m_prevTimeStamp(wxLog::GetTimestamp())
  {
    wxLog::SetTimestamp("");
  }

  ~ScopedErrorLogImpl(){
    wxLog::SetActiveTarget(m_prevTarget);
    wxLog::SetTimestamp(m_prevTimeStamp);
  }
  std::stringstream m_stream;
  wxLogStream m_logTarget;
  wxLog* m_prevTarget;
  wxString m_prevTimeStamp;
};

ScopedErrorLog::ScopedErrorLog(){
  m_impl = new ScopedErrorLogImpl;
}

ScopedErrorLog::~ScopedErrorLog(){
  delete m_impl;
}

utf8_string ScopedErrorLog::GetMessages() const {
  // Convert the messages to a wxString first, to hopefully interpret
  // the streamed bytes as the correct character encoding.
  // For example to support localized OS-errors, like
  // "Permission denied" = "&aring;tkomst nekad" in Swedish.
  wxString s(m_impl->m_stream.str());
  return utf8_string(s.utf8_str());
}

} // namespace
