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

#include <cassert>
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh" // subbitmap
#include "commands/bitmap-cmd.hh"
#include "commands/command.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"
#include "util/raster-selection.hh"
#include "util/default-settings.hh"

namespace faint{

class BmpTargetBase : public Command{
public:
  BmpTargetBase(CommandType type, BitmapCommand* cmd)
    : Command(type),
      m_cmd(cmd)
  {}

  ~BmpTargetBase(){
    delete m_cmd;
  }

  void Do(CommandContext& ctx) override final {
    DoBmp(m_cmd, ctx);
  }

  void Undo(CommandContext& ctx) override final{
    UndoBmp(ctx);
  }

  utf8_string Name() const override final{
    return m_cmd->Name();
  }

private:
  virtual void DoBmp(BitmapCommand*, CommandContext&) = 0;
  virtual void UndoBmp(CommandContext&) = 0;
  BitmapCommand* m_cmd;
};

class BmpTargetImage : public BmpTargetBase {
public:
  BmpTargetImage(BitmapCommand* cmd)
    : BmpTargetBase(CommandType::RASTER, cmd)
  {}
private:
  void DoBmp(BitmapCommand* cmd, CommandContext& ctx) override{
    cmd->Do(ctx.GetRawBitmap());
  }

  void UndoBmp(CommandContext&) override{
    // Raster background undo is managed elsewhere
  }
};

class BmpTargetRectangle : public BmpTargetBase {
public:
  BmpTargetRectangle(BitmapCommand* cmd, const IntRect& rect)
    : BmpTargetBase(CommandType::RASTER, cmd),
      m_rect(rect)
  {}
private:
  void DoBmp(BitmapCommand* cmd, CommandContext& ctx) override{
    Bitmap bmp(subbitmap(ctx.GetRawBitmap(), m_rect));
    cmd->Do(bmp);
    FaintDC& dc(ctx.GetDC());
    dc.Blit(bmp, floated(m_rect.TopLeft()), default_bitmap_settings());
  }

  void UndoBmp(CommandContext&) override{
    // Raster background undo is managed elsewhere
  }

  IntRect m_rect;
};

class BmpTargetSelection : public BmpTargetBase {
public:
  BmpTargetSelection(BitmapCommand* cmd)
    : BmpTargetBase(CommandType::SELECTION, cmd)
  {}
private:
  void DoBmp(BitmapCommand* cmd, CommandContext& ctx) override{
    RasterSelection& selection = ctx.GetRasterSelection();
    assert(!selection.Empty());
    if (selection.Floating()){
      Bitmap bmp(selection.GetBitmap());
      if (m_old.NotSet()){
        m_old.Set(bmp);
      }
      cmd->Do(bmp);
      selection.SetFloatingBitmap(bmp, selection.TopLeft());
    }
  }

  void UndoBmp(CommandContext& ctx) override{
    RasterSelection& selection = ctx.GetRasterSelection();
    assert(!selection.Empty());
    if (selection.Floating()){
      selection.SetFloatingBitmap(m_old.Get(), selection.TopLeft());
    }
  }

  // The old floating bitmap for undo.
  Optional<Bitmap> m_old;
};

Command* target_full_image(BitmapCommand* bmpCmd){
  return new BmpTargetImage(bmpCmd);
}

Command* target_floating_selection(BitmapCommand* bmpCmd){
  return new BmpTargetSelection(bmpCmd);
}

Command* target_rectangle(BitmapCommand* bmpCmd, const IntRect& rect){
  return new BmpTargetRectangle(bmpCmd, rect);
}

} // namespace
