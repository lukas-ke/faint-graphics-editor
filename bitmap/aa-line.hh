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

#ifndef FAINT_AA_LINE_HH
#define FAINT_AA_LINE_HH

namespace faint{

class Bitmap;
class IntLineSegment;
class ColRGB;

// Draws an anti-aliased line using Xiaolin Wu's line algorithm.
// Implemented from this pseudo-code:
// http://en.wikipedia.org/wiki/Xiaolin_Wu's_line_algorithm
void draw_line_aa_Wu(Bitmap&, const IntLineSegment&, const ColRGB&);

} // namespace

#endif
