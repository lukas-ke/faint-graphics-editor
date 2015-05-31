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

#ifndef FAINT_FAINT_COMMON_CURSORS_HH
#define FAINT_FAINT_COMMON_CURSORS_HH
#include "wx/cursor.h"
#include "gui/common-cursors.hh"

class wxWindow;

namespace faint{

class FaintCommonCursors final : public CommonCursors{
 public:
  FaintCommonCursors(const wxCursor& blank,
    const wxCursor& crosshair);
  FaintCommonCursors(const FaintCommonCursors&) = delete;

  void SetBlank(wxWindow*) const override;
  void SetCrosshair(wxWindow*) const override;

private:
  wxCursor m_blank;
  wxCursor m_crosshair;
};

} // namespace

#endif
