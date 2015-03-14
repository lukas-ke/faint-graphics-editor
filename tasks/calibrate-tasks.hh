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

#ifndef FAINT_CALIBRATE_TASKS_HH
#define FAINT_CALIBRATE_TASKS_HH

namespace faint{

class LineSegment;
class PosInfo;
class Task;

Task* calibrate_idle();
Task* calibrate_draw_line(const PosInfo&);
Task* calibrate_enter_measure(const LineSegment&, const PosInfo&);

} // namespace

#endif
