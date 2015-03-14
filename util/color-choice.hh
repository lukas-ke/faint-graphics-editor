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

#ifndef FAINT_COLOR_CHOICE_HH
#define FAINT_COLOR_CHOICE_HH
#include <utility>
#include "bitmap/paint.hh"

namespace faint{
class Settings;

using FlaggedPaint = std::pair<Paint, bool>;
class ColorChoice{
  // A foreground and background Paint, with a bool each
  // signifying if the color was from some primary Settings (true) or
  // from a fallback Settings object (false).
  //
  // In practice, this is used to know if the color targets a
  // selection or if it refers to the regular color choice.
public:
  ColorChoice(const FlaggedPaint& fg,
    const FlaggedPaint& bg);

  const FlaggedPaint bg;
  const FlaggedPaint fg;

  ColorChoice& operator=(const ColorChoice&) = delete;
};

// Returns a ColorChoice with the fg and bg flags set to true if
// they're in the preferred settings parameter, and indicatePreferred
// is true
ColorChoice get_color_choice(const Settings& preferred,
  const Settings& fallback,
  bool indicatePreferred);

} // namespace

#endif
