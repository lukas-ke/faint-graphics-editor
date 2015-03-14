// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_VISIT_SELECTION_HH
#define FAINT_VISIT_SELECTION_HH
#include "util/raster-selection.hh"

namespace faint{namespace sel{

template<typename EMPTY_FUNC,
  typename RECT_FUNC,
  typename MOVING_FUNC,
  typename COPYING_FUNC>
auto visit(const RasterSelection& s,
  const EMPTY_FUNC& empty,
  const RECT_FUNC& rect,
  const MOVING_FUNC& moving,
  const COPYING_FUNC& copying)
{
  if (s.Empty()){
    return empty(Empty());
  }
  else if (!s.Floating()){
    return rect(Rectangle(s.GetRect(), s.GetOptions()));
  }
  else if (s.Floating() && !s.Copying()){
    return moving(Moving(s.GetRect(),
      s.GetOldRect(),
      s.GetBitmap(),
      s.GetOptions()));
  }
  else{
    return copying(Copying(s.GetRect(), s.GetBitmap(), s.GetOptions()));
  }
}

template<typename EMPTY_FUNC, typename RECT_FUNC, typename FLOATING_FUNC>
auto visit(const RasterSelection& s,
  const EMPTY_FUNC& empty,
  const RECT_FUNC& rect,
  const FLOATING_FUNC& floating)
{
  if (s.Empty()){
    return empty(Empty());
  }
  else if (s.Floating()){
    return floating(Floating(s.GetRect(), s.GetBitmap(), s.GetOptions()));
  }
  else{
    return rect(Rectangle(s.GetRect(), s.GetOptions()));
  }
}

template<typename RECT_FUNC, typename MOVING_FUNC, typename COPYING_FUNC>
auto visit(const Existing& e,
  const RECT_FUNC& rect,
  const MOVING_FUNC& moving,
  const COPYING_FUNC& copying)
{
  const RasterSelection& s = e.Get();
  assert(!s.Empty());

  if (s.Floating()){
    if (s.Copying()){
      return copying(Copying(s.GetRect(), s.GetBitmap(), s.GetOptions()));
    }
    return moving(Moving(s.GetRect(),
      s.GetOldRect(),
      s.GetBitmap(),
      s.GetOptions()));
  }
  else{
    return rect(Rectangle(s.GetRect(), s.GetOptions()));
  }
}

}} // namespace

#endif
