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

#include "app/canvas.hh"
#include "app/faint-frame-context.hh"
#include "commands/frame-cmd.hh"
#include "util/image.hh"

namespace faint{

FaintFrameContext::FaintFrameContext(const Getter<Canvas&>& canvas)
  : m_canvas(canvas)
{}

void FaintFrameContext::AddFrame(){
  auto& canvas = m_canvas();
  canvas.RunCommand(add_frame_command(canvas.GetSize()));
}

void FaintFrameContext::CopyFrame(const OldIndex& src, const NewIndex& dst){
  auto& canvas = m_canvas();
  canvas.RunCommand(add_frame_command(canvas.GetFrame(src.Get()), dst.Get()));
}

Index FaintFrameContext::GetNumFrames() const{
  return m_canvas().GetNumFrames();
}

Index FaintFrameContext::GetSelectedFrame() const{
  return m_canvas().GetSelectedFrame();
}

void FaintFrameContext::MoveFrame(const OldIndex& src, const NewIndex& dst){
  m_canvas().RunCommand(reorder_frame_command(dst, src));
}

void FaintFrameContext::RemoveFrame(const Index& i){
  m_canvas().RunCommand(remove_frame_command(i));
}

void FaintFrameContext::SelectFrame(const Index& i){
  m_canvas().SelectFrame(i);
}

} // namespace
