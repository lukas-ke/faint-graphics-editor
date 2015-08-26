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

#ifndef FAINT_OBJSPLINE_HH
#define FAINT_OBJSPLINE_HH
#include <vector>

namespace faint{

class Object;
class Points;
class Settings;

Object* create_spline_object(const Points&, const Settings&);
bool is_spline(const Object&);
std::vector<Point> get_spline_points(const Object&);
std::vector<PathPt> spline_to_svg_path(const std::vector<Point>&);

} // namespace

#endif
