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

#ifndef FAINT_PY_FRAME_PROPS_HH
#define FAINT_PY_FRAME_PROPS_HH
#include "python/py-image-props.hh"
#include "util/index.hh"

namespace faint{

class FrameProps;
class ImageProps;

extern PyTypeObject FramePropsType;

class BoundFrameProps{
public:
  BoundFrameProps(ImageProps&, FrameProps&);
  ImageProps& image;
  FrameProps& frame;

  BoundFrameProps& operator=(const BoundFrameProps&) = delete;
};

struct framePropsObject{
  PyObject_HEAD
  imagePropsObject* imageProps;
  Index frame_index;
};

} // namespace

#endif
