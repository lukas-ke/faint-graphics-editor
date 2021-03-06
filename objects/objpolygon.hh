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

#ifndef FAINT_OBJPOLYGON_HH
#define FAINT_OBJPOLYGON_HH
#include <vector>
#include "objects/object-ptr.hh"

namespace faint{

class Object;
class Points;
class Settings;

Object* create_polygon_object_raw(const Points&, const Settings&);
ObjectPtr create_polygon_object(const Points&, const Settings&);

bool is_polygon(const Object*);
bool is_polygon(const Object&);
std::vector<Point> get_polygon_vertices(const Object*);
std::vector<Point> get_polygon_vertices(const Object&);

} // namespace

#endif
