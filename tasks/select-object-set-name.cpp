// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "app/active-canvas.hh"
#include "app/get-app-context.hh"
#include "commands/command.hh"
#include "commands/set-object-name-cmd.hh"
#include "editors/text-entry-util.hh"
#include "geo/geo-func.hh"
#include "geo/rect.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "rendering/overlay.hh"
#include "tasks/select-object-set-name.hh"
#include "tasks/task.hh"
#include "text/text-buffer.hh"
#include "tools/tool.hh" // Fixme: For TextContext et al., move to new file
#include "util/pos-info.hh"
#include "util-wx/key-codes.hh"

namespace faint{

static Optional<Object*> get_namee(const ActiveCanvas& canvas){
  const objects_t& objects = canvas->GetObjectSelection();
  if (objects.size() == 1){
    return option(objects.back());
  }
  return no_option();
}

static bool valid_name(const utf8_string&){
  return true; // Fixme: Extend, comply with xml NCNAME?
}

class SelectObjectSetName : public Task,
                            public TextContext,
                            public SelectionContext{
public:
  SelectObjectSetName(const ActiveCanvas& canvas) :
    m_active(false),
    m_canvas(canvas)
  {
    get_namee(m_canvas).Visit(
      [&](const Object* object){
        m_active = true;
        get_app_context().BeginTextEntry();
        object->GetName().Visit(
          [&](const utf8_string& s){
            m_text.set(s);
            m_text.caret(m_text.size());
          });
      });
  }

  ~SelectObjectSetName() override{
    if (m_active){
      get_app_context().EndTextEntry();
      m_active = false;
    }
  }

  void Activate() override{
  }

  bool AcceptsPastedText() const override{
    return true;
  }

  TaskResult Char(const KeyInfo& info) override{
    if (info.key.Is(key::esc)){
      // Cancel on escape
      return TaskResult::CHANGE;
    }
    else if (info.key.Is(key::enter)){
      return Commit();
    }
    else if (handle_key_press(info.key, m_text)){
      return TaskResult::DRAW;
    }
    return TaskResult::NONE;
  }

  TaskResult Commit() /* not override */{
    utf8_string name = m_text.get();
    if (!valid_name(name)){
      return TaskResult::DRAW;
    }
    return get_namee(m_canvas).Visit(
      [&](Object* namee){
        Optional<utf8_string> newName(name, !name.empty());
        if (namee->GetName() == newName){
          // Name unchanged
          return TaskResult::CHANGE;
        }
        else {
          m_command.Set(set_object_name_command(namee, newName));
          return TaskResult::COMMIT_AND_CHANGE;
        }
      },
      [](){
        return TaskResult::CHANGE;
      });
  }

  Optional<utf8_string> CopyText() const override{
    utf8_string s = m_text.get_selection();
    if (s.empty()){
      return {};
    }
    else{
      return Optional<utf8_string>(s);
    }
  }

  Optional<utf8_string> CutText() override{
    utf8_string s = m_text.get_selection();
    if (s.empty()){
      return {};
    }
    else{
      m_text.del();
      return Optional<utf8_string>(s);
    }
  }

  bool Delete() override{
    m_text.del();
    return true;
  }

  bool Deselect() override{
    m_text.select_none();
    return true;
  }

  TaskResult DoubleClick(const PosInfo&) override{
    return TaskResult::NONE;
  }

  void Draw(FaintDC&, Overlays& overlays, const PosInfo&) override{
    if (m_active){
      const objects_t& objects = m_canvas->GetObjectSelection();
      if (objects.size() == 1){
        overlays.Textbox(center_point(objects.front()->GetTri()), m_text,
          "Name");
      }
    }
  }

  bool DrawBeforeZoom(Layer) const override{
    return false;
  }

  bool EatsSettings() const override{
    return false;
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo&) const override{
    return Cursor::CARET; // Fixme: Only when over textbox
  };

  Task* GetNewTask() override{
    return nullptr;
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    const objects_t& objects = m_canvas->GetObjectSelection();
    if (objects.size() != 1){
      return {};
    }
    return floiled(inflated(bounding_rect(objects.front()->GetTri()),
      100, 10));
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return {};
  }

  TaskResult MouseDown(const PosInfo&) override{
    return Commit();
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo&) override{
    return TaskResult::NONE;
  }

  void Paste(const utf8_string& str) override{
    m_text.insert(str);
  }

  virtual TaskResult Preempt(const PosInfo&) override{
    return Commit();
  }

  bool RefreshOnMouseOut() const override{
    return false;
  }

  bool SelectAll() override{
    m_text.select(m_text.all());
    return true;
  }

  void SelectionChange() override{
  }

  Optional<const faint::SelectionContext&> SelectionContext() const override{
    return Optional<const faint::SelectionContext&>(*this);
  }

  void SetLayer(Layer) override{
  }

  Optional<const faint::TextContext&> TextContext() const override{
    return Optional<const faint::TextContext&>(*this);
  }

  void UpdateSettings() override{
  }

  SelectObjectSetName& operator=(const SelectObjectSetName&) = delete;

private:
  bool m_active;
  const ActiveCanvas& m_canvas;
  PendingCommand m_command;
  TextBuffer m_text;
};

Task* select_object_set_name(const ActiveCanvas& canvas){
  return new SelectObjectSetName(canvas);
}

} // namespace
