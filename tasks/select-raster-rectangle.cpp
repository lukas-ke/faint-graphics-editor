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
#include "app/active-canvas.hh"
#include "commands/command.hh"
#include "commands/set-raster-selection-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/measure.hh"
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "tasks/select-raster-base.hh"
#include "tasks/select-raster-rectangle.hh"
#include "text/formatting.hh"
#include "util/default-settings.hh"
#include "util/image-util.hh"
#include "util/pos-info.hh"
#include "util/tool-util.hh"

namespace faint{

static Point get_pos(const Point& anchor, const PosInfo& info){
  return info.modifiers.Secondary() ?
    constrain_to_square(anchor, info.pos, false) :
    info.pos;
}

class RasterSelectionRectangle : public RasterSelectionTask {
public:
  RasterSelectionRectangle(const Point& startPos,
    Settings& s,
    bool appendCommand,
    const ActiveCanvas& canvas)
    : m_appendCommand(appendCommand),
      m_canvas(canvas),
      m_maxDistance(0.0),
      m_p0(startPos),
      m_p1(startPos),
      m_settings(s)
  {}

  void Draw(FaintDC& dc, Overlays&, const PosInfo&) override{
    dc.Rectangle(tri_from_rect(Rect(m_p0, m_p1)), selection_rectangle_settings());
  }

  bool DrawBeforeZoom(Layer) const override{
    return true;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CROSSHAIR;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return IntRect(floored(m_p0), floored(m_p1));
  }

  TaskResult MouseDown(const PosInfo&) override{
    // Rectangle selection is already active after construction
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo& info) override{
    if (m_maxDistance < 1.0){
      return TaskResult::CHANGE;
    }
    else {
      IntRect rect(intersection(IntRect(floored(m_p0), floored(m_p1)),
        image_rect(info.canvas.GetImage())));

      if (m_oldOptions.IsSet()){
        m_command.Set(set_raster_selection_command(
          New(SelectionState(rect)),
          Old(SelectionState()),
          "Select Rectangle",
          m_appendCommand,
          New(raster_selection_options(m_settings)),
          Old(m_oldOptions.Get())));
      }
      else{
        m_command.Set(set_raster_selection_command(New(SelectionState(rect)),
          Old(SelectionState()),
          "Select Rectangle",
          m_appendCommand));
      }
      return TaskResult::COMMIT_AND_CHANGE;
    }
  }

  TaskResult MouseMove(const PosInfo& info) override{
    m_p1 = get_pos(m_p0, info);
    m_maxDistance = std::max(m_maxDistance, distance(m_p0, m_p1));
    info.status.SetText(str_from_to(m_p0, m_p1), 0);
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::CHANGE;
  }

  void SetOldOptions(const OldSelectionOptions& options){
    m_oldOptions.Set(options.Get());
  }

  RasterSelectionRectangle& operator=(const RasterSelectionRectangle&) = delete;

private:
  Canvas& GetCanvas() const override{
    return *m_canvas;
  }

  const Settings& GetSettings() override{
    return m_settings;
  }

  bool m_appendCommand;
  ActiveCanvas m_canvas;
  PendingCommand m_command;
  coord m_maxDistance;
  Point m_p0;
  Point m_p1;
  Settings& m_settings;
  Optional<SelectionOptions> m_oldOptions;
};

Task* raster_selection_rectangle_task(const Point& startPos,
  Settings& settings,
  bool appendCommand,
  const ActiveCanvas& canvas)
{
  return new RasterSelectionRectangle(startPos, settings, appendCommand,
    canvas);
}

Task* raster_selection_rectangle_task(const Point& startPos,
  Settings& settings,
  const OldSelectionOptions& options,
  bool appendCommand,
  const ActiveCanvas& canvas)
{
 RasterSelectionRectangle* task = new RasterSelectionRectangle(startPos,
   settings, appendCommand, canvas);
  task->SetOldOptions(options);
  return task;
}

} // namespace
