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

#include "app/faint-common-cursors.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

FaintCommonCursors::FaintCommonCursors(const wxCursor& blank,
  const wxCursor& crosshair)
  : m_blank(blank),
    m_crosshair(crosshair)
{}

void FaintCommonCursors::SetBlank(wxWindow* w) const{
  set_cursor(w, m_blank);
}

void FaintCommonCursors::SetCrosshair(wxWindow* w) const{
  set_cursor(w, m_crosshair);
}

} // namespace
