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

#include <cmath>
#include "app/canvas.hh"
#include "commands/tri-cmd.hh"
#include "geo/line.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "objects/object.hh"
#include "rendering/overlay.hh"
#include "tasks/select-object-resize.hh"
#include "tasks/standard-task.hh"
#include "text/formatting.hh"
#include "util/convenience.hh"
#include "util/grid.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"

namespace faint{

static void set_if_valid(Object* obj, const Tri& tri){
  if (valid(tri)){
    obj->SetTri(tri);
  }
}

class ResizeObjectBase : public StandardTask {
public:
  ResizeObjectBase(Object* object)
    : m_object(object),
      m_oldTri(object->GetTri())
  {
    m_object->SetActive();
    // Fixme: Find out a way to set ts_BoundedText = true
    // to text objects as a command, so that when resizing
    // an auto-bounded text object, it gets a size-box.
  }

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) final override{
    if (m_object->ShowSizeBox()){
      DrawOverlay(overlays);
    }
  }

  bool DrawBeforeZoom(Layer) const final override{
    return false;
  }

  Command* GetCommand() final override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const final override{
    return Cursor::ARROW;
  }

  Task* GetNewTask() final override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const final override{
    return inflated(m_object->GetRefreshRect(), info.objectHandleWidth);
  }

  TaskResult MouseDown(const PosInfo&) final override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) final override{
    return Commit();
  }

  TaskResult Preempt(const PosInfo&) final override{
    return Commit();
  }
protected:
  PendingCommand m_command;
  Object* m_object;
  const Tri m_oldTri;

private:
  ResizeObjectBase& operator=(const ResizeObjectBase&);
  TaskResult Commit(){
    m_object->ClearActive();
    m_command.Set(new TriCommand(m_object, New(m_object->GetTri()),
      Old(m_oldTri),
      "Resize"));
    return TaskResult::COMMIT_AND_CHANGE;
  }
  virtual void DrawOverlay(Overlays&) = 0;
};

class ResizeObjectTask : public ResizeObjectBase {
public:
  ResizeObjectTask(Object* object, Handle handle)
    : ResizeObjectBase(object),
      m_lockX(false),
      m_lockY(false),
      m_p0(0,0),
      m_origin(0,0)
  {
    Rect objRect(bounding_rect(m_oldTri));
    if (handle == Handle::P0){
      m_origin = objRect.BottomRight();
      m_p0 = objRect.TopLeft();
    }
    else if (handle == Handle::P1){
      m_origin = objRect.BottomLeft();
      m_p0 = objRect.TopRight();
    }
    else if (handle == Handle::P2){
      m_origin = objRect.TopRight();
      m_p0 = objRect.BottomLeft();
    }
    else if (handle == Handle::P3){
      m_origin = objRect.TopLeft();
      m_p0 = objRect.BottomRight();
    }
    else if (handle == Handle::P0P2){
      m_p0 = Point(objRect.Left(), objRect.Center().y);
      m_origin = Point(objRect.Right(), objRect.Center().y);
      m_lockY = true;
    }
    else if (handle == Handle::P1P3){
      m_p0 = Point(objRect.Right(), objRect.Center().y);
      m_origin = Point(objRect.Left(), objRect.Center().y);
      m_lockY = true;
    }
    else if (handle == Handle::P0P1){
      m_p0 = Point(objRect.Center().x , objRect.y);
      m_origin = Point(objRect.Center().x, objRect.Bottom());
      m_lockX = true;
    }
    else if (handle == Handle::P2P3){
      m_p0 = Point(objRect.Center().x, objRect.Bottom());
      m_origin = Point(objRect.Center().x , objRect.y);
      m_lockX = true;
    }
  }

  void DrawOverlay(Overlays& overlays) override{
    overlays.Rectangle(bounding_rect(m_object->GetTri()));
  }

  TaskResult MouseMove(const PosInfo& info) override{
    // Fixme: This should preferably not be done by scaling, since
    // this makes growing something at width 0 impossible.
    Point p = info.pos;
    if (info.modifiers.Primary()){
      objects_t objects = info.canvas.GetObjects();
      remove(m_object, from(objects));
      if (m_lockY){
        Rect oldRect(bounding_rect(m_oldTri));
        p.x = snap_x(p.x, objects, info.canvas.GetGrid(), oldRect.Top(),
          oldRect.Bottom());
      }
      else if (m_lockX){
        Rect oldRect(bounding_rect(m_oldTri));
        p.y = snap_y(p.y, objects, info.canvas.GetGrid(), oldRect.Left(),
          oldRect.Right());
      }
      else {
        p = snap(p, objects, info.canvas.GetGrid());
      }
    }

    const bool constrain = info.modifiers.Secondary() &&
      neither(m_lockX, m_lockY);

    const Scale scale = GetScale(p, constrain);
    set_if_valid(m_object, scaled(m_oldTri, scale, m_origin));
    info.status.SetText(str(scale));
    return TaskResult::DRAW;
  }

  ResizeObjectTask& operator=(const ResizeObjectTask&) = delete;

private:
  Scale GetScale(const Point& p, bool constrain){
    Point delta(m_p0 - m_origin);
    Scale scale(
      (m_lockX || delta.x == 0)? 1.0 : (p.x - m_origin.x) / delta.x,
      (m_lockY || delta.y == 0)? 1.0 : (p.y - m_origin.y) / delta.y);

    if (constrain){
     if (std::fabs(scale.x) > std::fabs(scale.y)){
       scale.y = ((scale.x < 0) == (scale.y < 0)) ? scale.x : -scale.x;
     }
     else{
       scale.x = ((scale.x < 0) == (scale.y < 0)) ? scale.y : -scale.y;
     }
    }
    return scale;
  }

  bool m_lockX;
  bool m_lockY;
  Point m_p0;
  Point m_origin;
};

class ResizeObjectAlignedTask : public ResizeObjectBase {
public:
  ResizeObjectAlignedTask(Object* object, Handle handle)
    : ResizeObjectBase(object),
      m_handle(handle)
  {}

  void DrawOverlay(Overlays& overlays) override{
    overlays.Parallelogram(m_object->GetTri());
  }

  TaskResult MouseMove(const PosInfo& info) override{
    Point p = info.pos;
    if (info.modifiers.Primary()){
      objects_t objects = info.canvas.GetObjects();
      remove(m_object, from(objects));
      p = snap(p, objects, info.canvas.GetGrid());
    }
    else if (info.modifiers.Secondary()){
      if (m_handle == Handle::P0 || m_handle == Handle::P3){
        p = projection(p, unbounded(LineSegment(m_oldTri.P0(), m_oldTri.P3())));
      }
      else if (m_handle == Handle::P1 || m_handle == Handle::P2){
        p = projection(p, unbounded(LineSegment(m_oldTri.P1(), m_oldTri.P2())));
      }
    }

    // Note: Incorrect when skewed
    if (m_handle == Handle::P0){
      Point p3 = m_oldTri.P3();
      Line p1p3 = unbounded(LineSegment(m_oldTri.P1(), p3));
      Line p2p3 = unbounded(LineSegment(m_oldTri.P2(), p3));
      Point p1 = projection(p, p1p3);
      Point p2 = projection(p, p2p3);
      set_if_valid(m_object, Tri(p, p1, p2));
    }
    else if (m_handle == Handle::P1){
      Point p2 = m_oldTri.P2();
      Line p0p2 = unbounded(LineSegment(m_oldTri.P0(), p2));
      Point p0 = projection(p, p0p2);
      set_if_valid(m_object, Tri(p0, p, p2));
    }
    else if (m_handle == Handle::P2){
      Point p1 = m_oldTri.P1();
      Line p0p1 = unbounded(LineSegment(m_oldTri.P0(), p1));
      Point p0 = projection(p, p0p1);
      set_if_valid(m_object, Tri(p0, p1, p));
    }
    else if (m_handle == Handle::P3){
      Point p0 = m_oldTri.P0();
      Line lp1 = unbounded(LineSegment(p0, m_oldTri.P1()));
      Line lp2 = unbounded(LineSegment(p0, m_oldTri.P2()));

      Point p1 = projection(p, lp1);
      Point p2 = projection(p, lp2);

      set_if_valid(m_object, Tri(p0, p1, p2));
    }
    else if (m_handle == Handle::P0P2){
      // Left side
      Line upper = unbounded(LineSegment(m_oldTri.P0(), m_oldTri.P1()));
      Line lower = unbounded(LineSegment(m_oldTri.P2(), m_oldTri.P3()));
      Point p0 = projection(p, upper);
      Point p2 = projection(p, lower);

      set_if_valid(m_object, Tri(p0, m_oldTri.P1(), p2));
    }
    else if (m_handle == Handle::P1P3){
      // Right side
      Line upper = unbounded(LineSegment(m_oldTri.P0(), m_oldTri.P1()));
      Point p1 = projection(p, upper);

      set_if_valid(m_object, Tri(m_oldTri.P0(), p1, m_oldTri.P2()));
    }
    else if (m_handle == Handle::P0P1){
      // Upper
      Line left = unbounded(LineSegment(m_oldTri.P2(), m_oldTri.P0()));
      Line right = unbounded(LineSegment(m_oldTri.P3(), m_oldTri.P1()));
      Point p0 = projection(p, left);
      Point p1 = projection(p, right);
      set_if_valid(m_object, Tri(p0, p1, m_oldTri.P2()));
    }
    else if (m_handle == Handle::P2P3){
      // Lower
      Line left = unbounded(LineSegment(m_oldTri.P2(), m_oldTri.P0()));
      Point p2 = projection(p, left);
      set_if_valid(m_object, Tri(m_oldTri.P0(), m_oldTri.P1(), p2));
    }

    return TaskResult::DRAW;
  }
private:
  ResizeObjectAlignedTask& operator=(const ResizeObjectAlignedTask&);
  Handle m_handle;
};

Task* select_object_resize(Object* obj, Handle handle){
  if (object_aligned_resize(obj)){
    return new ResizeObjectAlignedTask(obj, handle);
  }
  return new ResizeObjectTask(obj, handle);
}

} // namespace
