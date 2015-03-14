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

#include <algorithm>
#include "commands/command.hh"
#include "commands/frame-cmd.hh"
#include "geo/int-point.hh"
#include "text/utf8-string.hh"
#include "util/frame-props.hh"
#include "util/image.hh"
#include "util/optional.hh"

namespace faint{

class AddFrameCommand : public Command {
public:
  AddFrameCommand(const IntSize& size)
    : Command(CommandType::FRAME)
  {
    ImageInfo info(size, create_bitmap(false));
    m_image = new Image(FrameProps(info));
  }

  AddFrameCommand(const Image& frame, const Index& index)
    : Command(CommandType::FRAME),
      m_image(new Image(frame)),
      m_index(index)
  {}

  ~AddFrameCommand(){
    delete m_image;
  }

  void Do(CommandContext& context) override{
    if (m_index.IsSet()){
      context.AddFrame(m_image, m_index.Get());
    }
    else {
      context.AddFrame(m_image);
    }
  }

  utf8_string Name() const override{
    return "Add Frame";
  }

  void Undo(CommandContext& context) override{
    context.RemoveFrame(m_image);
  }

private:
  Image* m_image;
  Optional<Index> m_index;
};

Command* add_frame_command(const IntSize& size){
  return new AddFrameCommand(size);
}

Command* add_frame_command(const Image& image, const Index& index){
  return new AddFrameCommand(image, index);
}

class RemoveFrameCommand : public Command {
public:
  RemoveFrameCommand(const Index& index)
    : Command(CommandType::FRAME),
      m_image(nullptr),
      m_index(index)
  {}

  void Do(CommandContext& ctx) override{
    if (m_image == nullptr){
      // Retrieve the image address on the first run for undo
      m_image = &ctx.GetFrame(m_index);
    }
    ctx.RemoveFrame(m_index);
  }

  void Undo(CommandContext& ctx) override{
    assert(m_image != nullptr);
    ctx.AddFrame(m_image, m_index);
  }

  utf8_string Name() const override{
    return "Remove Frame";
  }
private:
  Image* m_image;
  Index m_index;
};

Command* remove_frame_command(const Index& index){
  return new RemoveFrameCommand(index);
}

static void swap_frames(CommandContext& ctx, const Index& f1, const Index& f2){
  Image* img1 = &ctx.GetFrame(f1);
  Image* img2 = &ctx.GetFrame(f2);
  ctx.RemoveFrame(f2);
  ctx.AddFrame(img2, Index(f1.Get() + 1));
  ctx.RemoveFrame(f1);
  ctx.AddFrame(img1, f2);
}

class SwapFramesCommand : public Command {
public:
  SwapFramesCommand(const Index& f1, const Index& f2)
    : Command(CommandType::OBJECT),
      m_f1(f1),
      m_f2(f2)
  {
    assert(m_f2.Get() > m_f1.Get());
  }

  void Do(CommandContext& ctx) override{
    swap_frames(ctx, m_f1, m_f2);
  }

  void Undo(CommandContext& ctx) override{
    swap_frames(ctx, m_f1, m_f2);
  }

  utf8_string Name() const override{
    return "Swap frames";
  }
private:
  Index m_f1;
  Index m_f2;
};

Command* swap_frames_command(const Index& f1, const Index& f2){
  return new SwapFramesCommand(std::min(f1, f2), std::max(f1, f2));
}

class ReorderFrameCommand : public Command {
public:
  ReorderFrameCommand(const NewIndex& newIndex, const OldIndex& oldIndex)
    : Command(CommandType::FRAME),
      m_newIndex(newIndex),
      m_oldIndex(oldIndex)
  {}

  void Do(CommandContext& ctx) override{
    ctx.ReorderFrame(m_newIndex, m_oldIndex);
  }

  void Undo(CommandContext& ctx) override{
    ctx.ReorderFrame(New(m_oldIndex.Get()), Old(m_newIndex.Get()));
  }

  utf8_string Name() const override{
    return "Reorder Frames";
  }
private:
  NewIndex m_newIndex;
  OldIndex m_oldIndex;
};

Command* reorder_frame_command(const NewIndex& newIndex,
  const OldIndex& oldIndex)
{
  return new ReorderFrameCommand(newIndex, oldIndex);
}

class SetFrameDelayCommand : public Command {
public:
  SetFrameDelayCommand(const Index& frameIndex,
    const NewDelay& newDelay,
    const OldDelay& oldDelay)
    : Command(CommandType::FRAME),
      m_frameIndex(frameIndex),
      m_newDelay(newDelay.Get()),
      m_oldDelay(oldDelay.Get())
  {}

  void Do(CommandContext& ctx) override{
    ctx.GetFrame(m_frameIndex).SetDelay(m_newDelay);
  }

  void Undo(CommandContext& ctx) override{
    ctx.GetFrame(m_frameIndex).SetDelay(m_oldDelay);
  }

  utf8_string Name() const override{
    return "Set Frame Delay";
  }
private:
  Index m_frameIndex;
  Delay m_newDelay;
  Delay m_oldDelay;
};

Command* set_frame_delay_command(const Index& frameIndex,
  const NewDelay& newDelay,
  const OldDelay& oldDelay)
{
  return new SetFrameDelayCommand(frameIndex, newDelay, oldDelay);
}

class SetFrameHotSpotCommand : public Command {
public:
  SetFrameHotSpotCommand(const Index& frameIndex,
    const NewHotSpot& newHotSpot,
    const OldHotSpot& oldHotSpot)
    : Command(CommandType::FRAME),
      m_frameIndex(frameIndex),
      m_newHotSpot(newHotSpot.Get()),
      m_oldHotSpot(oldHotSpot.Get())

  {}

  void Do(CommandContext& ctx) override{
    ctx.GetFrame(m_frameIndex).SetHotSpot(m_newHotSpot);
  }

  void Undo(CommandContext& ctx) override{
    ctx.GetFrame(m_frameIndex).SetHotSpot(m_oldHotSpot);
  }

  utf8_string Name() const override{
    return "Set Frame HotSpot";
  }
private:
  Index m_frameIndex;
  HotSpot m_newHotSpot;
  HotSpot m_oldHotSpot;
};

Command* set_frame_hotspot_command(const Index& frameIndex,
  const NewHotSpot& newHotSpot,
  const OldHotSpot& oldHotSpot)
{
  return new SetFrameHotSpotCommand(frameIndex, newHotSpot, oldHotSpot);
}

} // namespace
