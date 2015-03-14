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

#include <cassert>
#include "commands/tri-cmd.hh"
#include "geo/angle.hh"
#include "geo/geo-list-points.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "objects/object.hh"
#include "rendering/overlay.hh"
#include "tasks/select-object-rotate.hh"
#include "tasks/standard-task.hh"
#include "text/formatting.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"

namespace faint{

// Returns the position of the opposite corner of the indicated handle
static Point pivot_from_handle(const Object* obj, Handle handle){
  Tri tri(obj->GetTri());
  if (object_aligned_resize(obj)){
    if (handle == Handle::P0){
      return tri.P3();
    }
    else if (handle == Handle::P1){
      return tri.P2();
    }
    else if (handle == Handle::P2){
      return tri.P1();
    }
    else if (handle == Handle::P3){
      return tri.P0();
    }
  }
  else{
    Rect rect(bounding_rect(tri));
    if (handle == Handle::P0){
      return rect.BottomRight();
    }
    else if (handle == Handle::P1){
      return rect.BottomLeft();
    }
    else if (handle == Handle::P2){
      return rect.TopRight();
    }
    else if (handle == Handle::P3){
      return rect.TopLeft();
    }
  }
  assert(false);
  return Point(0,0);
}

static Point origin_from_handle(Object* obj, Handle handle){
  auto pts = object_aligned_resize(obj) ?
    points_number_order(obj->GetTri()) :
    corners(bounding_rect(obj->GetTri()));
  return pts[static_cast<size_t>(handle)];
}

class RotateObjectTask : public StandardTask {
public:
  RotateObjectTask(Object* obj, Handle handle)
    : m_handle(origin_from_handle(obj, handle)),
      m_object(obj),
      m_oldTri(obj->GetTri()),
      m_pivot(pivot_from_handle(obj, handle))
  {
    m_object->SetActive();
  }


  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    overlays.Pivot(m_pivot);
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::ROTATE;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    return inflated(m_object->GetRefreshRect(), info.objectHandleWidth);
  }

  TaskResult MouseDown(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return Commit();
  }

  TaskResult MouseMove(const PosInfo& info) override{
    const Point& pos = info.pos;
    Angle a1 = line_angle({m_pivot, m_handle});
    Angle a2 = line_angle({m_pivot, pos});
    Angle delta = a2 - a1;
    if (info.modifiers.Secondary()){
      if (delta > -pi/4.0 && delta < pi/4.0){
        delta = Angle::Zero();
      }
      else if (delta > pi / 4.0 && delta < (pi * 3.0 / 4.0)){
        delta = pi / 2;
      }
      else if (delta > (pi * 3/4.0) && delta < (pi * 3.0 / 4.0) + pi / 4.0){
        delta = pi;
      }
      else {
        delta = pi + pi / 2.0;
      }
    }

    info.status.SetText(str_degrees_symbol(delta));

    Tri t2(rotated(m_oldTri, delta, m_pivot));
    m_object->SetTri(t2);
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return Commit();
  }

private:
  TaskResult Commit(){
    m_object->ClearActive();
    m_command.Set(new TriCommand(m_object, New(m_object->GetTri()),
      Old(m_oldTri), "Rotate"));
    return TaskResult::COMMIT_AND_CHANGE;
  }

  PendingCommand m_command;
  Point m_handle;
  Object* m_object;
  Tri m_oldTri;
  Point m_pivot;
};

Task* rotate_object_task(Object* obj, Handle handle){
  return new RotateObjectTask(obj, handle);
}

} // namespace
