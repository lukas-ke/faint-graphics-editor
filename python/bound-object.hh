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

#ifndef FAINT_BOUND_OBJECT_HH
#define FAINT_BOUND_OBJECT_HH
#include <vector>
#include "util/id-types.hh"
#include "util/objects.hh"

namespace faint{

class PyFuncContext;
class Canvas;
class Object;

template<class T>
class BoundObject{
  // Connects an Object to the id of the frame that contains it, and
  // to the canvas containing that frame. Used as the target for
  // MappedType<smthObject*>::GetCppObject.
public:
  BoundObject(PyFuncContext* ctx, Canvas* canvas, T* obj, FrameId frameId)
    : canvas(canvas),
      ctx(ctx),
      obj(obj),
      frameId(frameId)
  {}
  BoundObject()
    : canvas(nullptr),
      ctx(nullptr),
      obj(nullptr),
      frameId(FrameId::Invalid())
  {}

  BoundObject<Object> Plain(){
    return BoundObject<Object>(ctx, canvas, obj, frameId);
  }

  Canvas* canvas;
  PyFuncContext* ctx;
  T* obj;
  FrameId frameId;
};

template<typename T>
BoundObject<T> bind_object(PyFuncContext& ctx,
  Canvas& canvas,
  T* object,
  FrameId frameId)
{
  return BoundObject<T>(&ctx, &canvas, object, frameId);
}

using BoundObjects = std::vector<BoundObject<Object>>;

BoundObjects bind_objects(PyFuncContext&,
  Canvas&,
  const objects_t&, FrameId);

} // namespace

#endif
