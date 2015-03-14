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

#include "util/color-choice.hh"
#include "util/setting-id.hh"
#include "util/settings.hh"

namespace faint{

ColorChoice::ColorChoice(const std::pair<Paint, bool>& fg,
  const std::pair<Paint, bool>& bg)
  : bg(bg),
    fg(fg)
{}

ColorChoice get_color_choice(const Settings& preferred,
  const Settings& fallback,
  bool indicatePreferred)
{
  return ColorChoice(preferred.Has(ts_Fg) ?
    std::make_pair(preferred.Get(ts_Fg), indicatePreferred) :
    std::make_pair(fallback.Get(ts_Fg), false),

    preferred.Has(ts_Bg) ?
    std::make_pair(preferred.Get(ts_Bg), indicatePreferred) :
    std::make_pair(fallback.Get(ts_Bg), false));
}

} // namespace
