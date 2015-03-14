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

#ifndef FAINT_FLIP_ROTATE_CMD_HH
#define FAINT_FLIP_ROTATE_CMD_HH
#include "util/distinct.hh"

namespace faint{

enum class Axis;
class Angle;
class Command;
class Scale;
class Paint;

using AltPaint = Alternative<Paint>;

Command* rotate_scale_image_bilinear_command(const Angle&,
  const Scale&, const Paint& bg);

Command* rotate_image_command(const Angle&, const Paint& bg);

Command* rotate_image_command(const Angle&, const Paint& bg,
  const AltPaint& altBg);

Command* rotate_image_90cw_command();

Command* flip_image_command(Axis);

} // namespace

#endif
