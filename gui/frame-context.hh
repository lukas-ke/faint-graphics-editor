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

#ifndef FAINT_FRAME_CONTEXT_HH
#define FAINT_FRAME_CONTEXT_HH
#include "geo/int-size.hh"
#include "util/index.hh"

namespace faint{

class FrameContext{
public:
  virtual ~FrameContext() = default;
  virtual void AddFrame() = 0;
  virtual void CopyFrame(const OldIndex&, const NewIndex&) = 0;
  virtual Index GetNumFrames() const = 0;
  virtual Index GetSelectedFrame() const = 0;
  virtual void MoveFrame(const OldIndex&, const NewIndex&) = 0;
  virtual void RemoveFrame(const Index&) = 0;
  virtual void SelectFrame(const Index&) = 0;
};

} // namespace

#endif
