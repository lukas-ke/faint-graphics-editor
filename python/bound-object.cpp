// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include "python/bound-object.hh"
#include "util/make-vector.hh"

namespace faint{

BoundObjects bind_objects(PyFuncContext& ctx,
  Canvas& canvas,
  const objects_t& objects,
  FrameId frameId)
{
  return make_vector(objects,
    [&](Object* o){
      return bind_object(ctx, canvas, o, frameId);
    });
}

} // namespace
