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

#include <memory>
#include "app/canvas.hh"
#include "commands/command.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/measure.hh"
#include "tasks/select-raster-base.hh"
#include "tasks/select-raster-move.hh"
#include "util/color-span.hh"
#include "util/image-util.hh"
#include "util/raster-selection.hh"
#include "util/pos-info.hh"

namespace faint{

class RasterSelectionMove : public RasterSelectionTask {
public:
  RasterSelectionMove(const IntPoint& offset,
    const IntPoint& topLeft,
    const copy_selected& copy,
    bool floatSelected,
    Settings& s,
    Canvas& canvas)
    : m_canvas(canvas),
      m_constrainDir(ConstrainDir::NONE),
      m_lastOrigin(topLeft),
      m_offset(offset),
      m_settings(s)
  {
    // Do all adjustments through a mirage. The final changes to the
    // real selection are done in a command.
    m_selection = std::make_shared<RasterSelection>(
      m_canvas.GetRasterSelection());
    assert(!m_selection->Empty());

    if (floatSelected){
      m_selection->Clip(image_rect(m_canvas.GetImage()));
      if (!m_selection->Empty()){
        m_canvas.GetBackground().Visit(
          [&](const Bitmap& bmp){
            m_selection->BeginFloat(bmp, copy);
          },
          [&](const ColorSpan& span){
            // Fixme: Handle OOM
            m_selection->BeginFloat(Bitmap(span.size, span.color), copy);
          });
      }
    }
    else {
      IntRect r = m_selection->GetRect();
      m_selection->Deselect();
      m_selection->SetRect(r);
    }
    m_canvas.SetMirage(m_selection);
  }

  void Draw(FaintDC&, Overlays&, const PosInfo&) override{
    // The currently modified selection is drawn by the mirage.
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::MOVE;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return m_lastRect;
  }

  TaskResult MouseDown(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo& info) override{
    m_command.Set(set_raster_selection_command(New(m_selection->GetState()),
        Old(info.canvas.GetRasterSelection().GetState()),
      m_selection->Floating() ?
        "Move Selected Content" : "Move Selection", true));
    return TaskResult::COMMIT_AND_CHANGE;
}

  TaskResult MouseMove(const PosInfo& info) override{
    if (m_selection->Empty()){
      return TaskResult::NONE;
    }

    IntPoint topLeft = floored(info.pos) - m_offset;
    if (info.modifiers.Secondary()){
      const coord lockDistance = 100.0;
      coord zoom = info.canvas.GetZoom();

      if (distance(topLeft, m_lastOrigin) * zoom > lockDistance){
        // Lock the constraining to a direction if the distance is
        // somewhat large, then prefer this direction when at lesser
        // distances, so that constraining can be used also close to
        // the original position after establishing a likely
        // intended-constrain direction.
        m_constrainDir = constrain_pos(topLeft, m_lastOrigin);
      }
      else if (m_constrainDir == ConstrainDir::NONE){
        constrain_pos(topLeft, m_lastOrigin);
      }
      else{
        constrain_pos(topLeft, m_lastOrigin, m_constrainDir);
      }
    }

    m_selection->Move(topLeft);
    m_lastRect = m_selection->GetRect();
    info.status.SetMainText("");
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::CHANGE;
  }

private:
  Canvas& GetCanvas() const override{
    return m_canvas;
  }

  const Settings& GetSettings() override{
    return m_settings;
  }

  Canvas& m_canvas;
  PendingCommand m_command;
  ConstrainDir m_constrainDir;
  IntPoint m_lastOrigin;
  IntRect m_lastRect;
  IntPoint m_offset;
  std::shared_ptr<RasterSelection> m_selection;
  Settings& m_settings;
};

Task* raster_selection_move_task(const IntPoint& offset,
  const IntPoint& topLeft,
  const copy_selected& copySelected,
  bool floatSelected,
  Settings& settings,
  Canvas& canvas)
{
  return new RasterSelectionMove(offset,
    topLeft,
    copySelected,
    floatSelected,
    settings,
    canvas);
}

} // namespace
