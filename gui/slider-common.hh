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

#ifndef FAINT_SLIDER_COMMON_HH
#define FAINT_SLIDER_COMMON_HH
#include <memory> // unique_ptr

class wxWindow;

namespace faint{

class Bitmap;
class ClosedIntRange;
class IntSize;

enum class SliderDir{HORIZONTAL, VERTICAL};

class SliderBackground{
  // Base class for slider background renderers.
public:
  virtual void Draw(Bitmap&, const IntSize&, SliderDir) = 0;
};

using SliderBackgroundPtr = std::unique_ptr<SliderBackground>;

// Background filled with a solid color.
SliderBackgroundPtr slider_bg_Rectangle();

// Background which indicates the middle of the slider range with a
// line.
SliderBackgroundPtr slider_bg_MidPoint();

class SliderCursors{
public:
  SliderCursors() = default;
  SliderCursors(const SliderCursors&) = delete;
  virtual ~SliderCursors() = default;

  virtual void SetHandleCursor(wxWindow*, SliderDir) const = 0;
  virtual void SetOffsetCursor(wxWindow*, SliderDir) const = 0;
};

double pos_to_value(const int pos, const int length, const ClosedIntRange&);
int value_to_pos(const double value, const int length, const ClosedIntRange&);

} // namespace

#endif
