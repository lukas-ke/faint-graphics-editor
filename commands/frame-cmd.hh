// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#ifndef FAINT_FRAME_CMD_HH
#define FAINT_FRAME_CMD_HH
#include "commands/command-ptr.hh"
#include "util/delay.hh"
#include "util/distinct.hh"
#include "util/hot-spot.hh"
#include "util/index.hh"

namespace faint{

class Image;
class Index;
class IntPoint;
class IntSize;

CommandPtr add_frame_command(const IntSize&);
CommandPtr add_frame_command(const Image&, const Index&);
CommandPtr remove_frame_command(const Index&);
CommandPtr reorder_frame_command(const NewIndex&, const OldIndex&);
CommandPtr swap_frames_command(const Index&, const Index&);

using NewDelay = Order<Delay>::New;
using OldDelay = Order<Delay>::Old;

CommandPtr set_frame_delay_command(const Index&,
  const NewDelay&,
  const OldDelay&);

using NewHotSpot = Order<HotSpot>::New;
using OldHotSpot = Order<HotSpot>::Old;

CommandPtr set_frame_hotspot_command(const Index& frameIndex,
  const NewHotSpot&,
  const OldHotSpot&);

} // namespace

#endif
