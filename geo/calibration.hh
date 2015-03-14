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

#ifndef FAINT_CALIBRATION_HH
#define FAINT_CALIBRATION_HH
#include "geo/line.hh"
#include "text/utf8-string.hh"

namespace faint{

class Calibration{
public:
  Calibration()
    : length(1.0)
  {}

  Calibration(const LineSegment& l, coord length, const utf8_string& unit)
    : pixelLine(l),
      length(length),
      unit(unit)
  {}

  coord Scale() const{
    return faint::length(pixelLine) / this->length;
  }

  LineSegment pixelLine;
  coord length;
  utf8_string unit;
};

} // namespace;

#endif
