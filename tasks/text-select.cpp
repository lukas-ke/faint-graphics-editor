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

#include "objects/objtext.hh"
#include "rendering/overlay.hh"
#include "tasks/standard-task.hh"
#include "tasks/text-select.hh"
#include "util/pos-info.hh"

namespace faint{

class SelectText : public StandardTask {
  // Task for marking text in a text object
public:
  SelectText(ObjText* obj, bool newTextObject, const Point& clickPos)
    : m_newTextObject(newTextObject),
      m_origin(clickPos),
      m_textObject(obj)
  {
    m_textObject->SetCaretPos(m_textObject->CaretPos(clickPos), false);
  }

  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override{
    overlays.Corners(m_textObject->GetTri());
    if (m_newTextObject){
      auto& ctx(get_expression_context(info));
      m_textObject->Draw(dc, ctx);
    }

    if (!m_textObject->HasSelectedRange()){
      overlays.Caret(m_textObject->GetCaret());
    }
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return layer == Layer::RASTER;
  }

  Command* GetCommand() override{
    return nullptr;
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CARET;
  }

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return m_textObject->GetRefreshRect();
  }

  TaskResult MouseDown(const PosInfo&) override{
    // The task is created on left down
    return TaskResult::NONE;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::CHANGE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    m_textObject->SetCaretPos(m_textObject->CaretPos(info.pos), true);
    return TaskResult::DRAW;
  }

  TaskResult Preempt(const PosInfo&) override{
    return TaskResult::NONE;
  }

  SelectText& operator=(const SelectText&) = delete;
private:
  bool m_newTextObject;
  Point m_origin;
  ObjText* m_textObject;
};

Task* select_text_task(ObjText* obj, bool newTextObject, const Point& clickPos){
  return new SelectText(obj, newTextObject, clickPos);
}

} // namespace
