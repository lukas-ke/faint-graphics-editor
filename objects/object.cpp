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

#include "bitmap/color.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "util/settings.hh"

namespace faint{

const Settings NullObjectSettings;

const Color mask_outside(255,255,255);
const Color mask_fill(0,255,0);
const Color mask_no_fill(0,0,255);
const Color mask_edge(255,0,0);

void Object::ClearActive(){
  SetActive(false);
}

bool Object::Inactive() const{
  return !Active();
}

Object* Object_clone(const Object* o){
  return o->Clone();
}

Tri Object_get_tri(const Object* o){
  return o->GetTri();
}

} // namespace
