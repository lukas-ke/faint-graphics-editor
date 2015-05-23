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

#ifndef FAINT_FAINT_SLIDER_CURSORS_HH
#define FAINT_FAINT_SLIDER_CURSORS_HH
#include "wx/cursor.h"
#include "gui/slider-common.hh"

namespace faint{

class FaintSliderCursors final : public SliderCursors{
public:
FaintSliderCursors(const wxCursor& horizontal,
    const wxCursor& vertical);
  // Fixme: Replace with Set(SliderDir, wxWindow*).
  void SetHorizontal(wxWindow*) const override;
  void SetVertical(wxWindow*) const override;

  FaintSliderCursors(const FaintSliderCursors&) = delete;
private:
  wxCursor m_horizontal;
  wxCursor m_vertical;
};

} // namespace

#endif
