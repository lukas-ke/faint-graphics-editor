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

#include "app/canvas.hh"
#include "geo/adjust.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "geo/measure.hh"
#include "geo/rect.hh"
#include "objects/object.hh"
#include "tasks/select-object-move.hh"
#include "tasks/standard-task.hh"
#include "util/command-util.hh"
#include "util/image.hh"
#include "util/grid.hh"
#include "util/object-util.hh"
#include "util/pos-info.hh"

namespace faint{

static Point get_delta(Object* obj, const Point& p){
  return p - obj->GetTri().P0();
}

class MoveObjectTask : public StandardTask {
public:
  MoveObjectTask(Object* mainObject,
    const objects_t& allObjects,
    const Point& offset,
    MoveMode mode)
    : m_copy(mode == MoveMode::COPY),
      m_mainObject(mainObject),
      m_moved(false),
      m_offset(offset),
      m_oldTri(mainObject->GetTri()),
      m_refreshRect(floated(mainObject->GetRefreshRect()))
  {
    for (Object* object : allObjects){
      m_refreshRect = union_of(m_refreshRect, floated(object->GetRefreshRect()));
      m_objects.push_back(object);
      m_origTris.push_back(object->GetTri());
      object->SetActive();
    }

    if (m_copy){
      // Set the pending command for copied objects so that the objects
      // are freed if the command is left unretrieved.
      m_command.Set(get_add_objects_command(m_objects,
        select_added(true), "Clone"));
    }

    m_refreshRect = inflated(m_refreshRect, 5, 5);
  }

  void Draw(FaintDC& dc, Overlays&, const PosInfo& info) override{
    if (m_copy){
      // Copied objects need to be drawn here, as they're not added to
      // the canvas yet (merely moved objects will be drawn by the
      // canvas).
      ExpressionContext& ctx(info.canvas.GetImage().GetExpressionContext());
      for (Object* object : m_objects){
        object->Draw(dc, ctx);
      }
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return m_copy ? Cursor::CLONE : Cursor::MOVE;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return floiled(m_refreshRect);
  }

  TaskResult MouseDown(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    for (Object* object : m_objects){
      object->ClearActive();
    }
    return Commit();
  }

  TaskResult MouseMove(const PosInfo& info) override{
    m_moved = true;
    Point pos(info.pos - m_offset);
    if (info.modifiers.Secondary()){
      if (object_aligned_resize(m_mainObject)){
        Point p1 = projection(pos, unbounded(LineSegment(m_oldTri.P0(),
          m_oldTri.P1())));
        Point p2 = projection(pos, unbounded(LineSegment(m_oldTri.P0(),
          m_oldTri.P2())));

        pos = distance(pos,p1) < distance(pos,p2) ? p1 : p2;
      }
      else{
        constrain_pos(pos, m_oldTri.P0());
      }
    }
    Point delta = get_delta(m_mainObject, pos);
    offset_by(m_objects, delta);
    m_refreshRect = translated(m_refreshRect, delta);

    if (info.modifiers.Primary()){
      Point snapOffset = SnapObject(info.canvas.GetObjects(),
        info.canvas.GetGrid());
      offset_by(m_objects, snapOffset);
      m_refreshRect = translated(m_refreshRect, snapOffset);
    }
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return Commit();
  }

private:
  TaskResult Commit(){
    if (!m_moved){
      return TaskResult::CHANGE;
    }
    if (m_copy){
      // When copying, the command is prepared by the constructor.
      return TaskResult::COMMIT_AND_CHANGE;
    }
    m_command.Set(get_move_objects_command(m_objects,
      New(get_tris(m_objects)), Old(m_origTris)));
    return TaskResult::COMMIT_AND_CHANGE;
  }

  Point SnapObject(objects_t snappable, const Grid& grid){
    std::vector<Point> points = m_mainObject->GetSnappingPoints();
    if (points.empty()){
      return Point(0,0);
    }
    // Do not snap to any of the moved objects
    remove(m_objects, from(snappable));
    Point p_first(points.front());
    Point p_adj = snap(p_first, snappable, grid);
    Point delta = p_adj - p_first;
    return delta;
  }

  PendingCommand m_command;
  bool m_copy;
  Object* m_mainObject;
  bool m_moved;
  objects_t m_objects;
  Point m_offset;
  Tri m_oldTri;
  std::vector<Tri> m_origTris;
  Rect m_refreshRect;
};

Task* move_object_task(Object* mainObject,
  const objects_t& allObjects,
  const Point& offset,
  MoveMode mode)
{
  return new MoveObjectTask(mainObject, allObjects, offset, mode);
}

} // namespace
