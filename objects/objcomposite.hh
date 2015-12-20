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

#ifndef FAINT_OBJCOMPOSITE_HH
#define FAINT_OBJCOMPOSITE_HH
#include "objects/object-ptr.hh"
#include "util/objects.hh"

namespace faint{

enum class Ownership{
  OWNER, // Composite should delete contained objects on destruction
  LOANER // ...should not
};

Object* create_composite_object_raw(const objects_t&, Ownership);
ObjectPtr create_composite_object(const objects_t&, Ownership);

bool is_composite(const Object&);

} // namespace

#endif
