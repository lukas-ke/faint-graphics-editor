// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_APPLY_TARGET_HH
#define FAINT_APPLY_TARGET_HH
#include "util/either.hh"

namespace faint{
class IntSize;
class Size;

using EitherSize = Either<IntSize, Size>;
enum class ApplyTarget{
  APPLY_OBJECT_SELECTION,
  APPLY_RASTER_SELECTION,
  APPLY_IMAGE
};

class OBJECT_SELECTION{};
class RASTER_SELECTION{};
class IMAGE{};

template<typename OBJECTF, typename RASTERF, typename IMAGEF>
auto dispatch_target(ApplyTarget t,
  OBJECTF objectFunc,
  RASTERF rasterFunc,
  IMAGEF imageFunc)
{
  switch(t){
  case ApplyTarget::APPLY_OBJECT_SELECTION:
    return objectFunc(OBJECT_SELECTION());
  case ApplyTarget::APPLY_RASTER_SELECTION:
    return rasterFunc(RASTER_SELECTION());
  case ApplyTarget::APPLY_IMAGE:
    return imageFunc(IMAGE());
  }
  assert(false);
  return imageFunc(IMAGE());
}

// Like the other dispatch_target, but requires specifying a return
// type (RET).
//
// This allows avoiding specifying the return type for the individual
// functors for cases when they return different - but convertible - types.
template<typename RET, typename OBJECTF, typename RASTERF, typename IMAGEF>
RET dispatch_target(ApplyTarget t,
  OBJECTF objectFunc,
  RASTERF rasterFunc,
  IMAGEF imageFunc)
{
  switch(t){
  case ApplyTarget::APPLY_OBJECT_SELECTION:
    return objectFunc(OBJECT_SELECTION());
  case ApplyTarget::APPLY_RASTER_SELECTION:
    return rasterFunc(RASTER_SELECTION());
  case ApplyTarget::APPLY_IMAGE:
    return imageFunc(IMAGE());
  }
  assert(false);
  return imageFunc(IMAGE());
}

class Canvas;
ApplyTarget get_apply_target(const Canvas&);
EitherSize get_apply_target_size(const Canvas&, ApplyTarget);

} // namespace

#endif
