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

#ifndef FAINT_FAINT_FRAME_CONTEXT_HH
#define FAINT_FAINT_FRAME_CONTEXT_HH
#include "gui/frame-context.hh"
#include "util/accessor.hh"

namespace faint{

class Canvas;

class FaintFrameContext : public FrameContext{
public:
  FaintFrameContext(const Getter<Canvas&>&);
  void AddFrame() override;
  void CopyFrame(const OldIndex&, const NewIndex&) override;
  Index GetNumFrames() const override;
  Index GetSelectedFrame() const override;
  void MoveFrame(const OldIndex&, const NewIndex&) override;
  void RemoveFrame(const Index&) override;
  void SelectFrame(const Index&) override;
private:
  Getter<Canvas&> m_canvas;
};

} // namespace

#endif
