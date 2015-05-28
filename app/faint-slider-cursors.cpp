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

#include "util-wx/fwd-wx.hh"
#include "app/faint-slider-cursors.hh"

namespace faint{

FaintSliderCursors::FaintSliderCursors(const wxCursor& horizontalHandle,
  const wxCursor& verticalHandle,
  const wxCursor& horizontalOffset,
  const wxCursor& verticalOffset)
  : m_horizontalHandle(horizontalHandle),
    m_horizontalOffset(horizontalOffset),
    m_verticalHandle(verticalHandle),
    m_verticalOffset(verticalOffset)
{}

void FaintSliderCursors::SetHandleCursor(wxWindow* w, SliderDir dir) const{
  set_cursor(w,
    dir == SliderDir::HORIZONTAL ? m_horizontalHandle : m_verticalHandle);
}

void FaintSliderCursors::SetOffsetCursor(wxWindow* w, SliderDir dir) const{
  set_cursor(w,
    dir == SliderDir::HORIZONTAL ? m_horizontalOffset : m_verticalOffset);
}

} // namespace
