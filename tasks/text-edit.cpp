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
#include <functional>
#include "commands/command-bunch.hh"
#include "commands/text-entry-cmd.hh"
#include "editors/text-entry-util.hh"
#include "geo/rect.hh"
#include "objects/objtext.hh"
#include "rendering/overlay.hh"
#include "tasks/task.hh"
#include "tasks/text-edit.hh"
#include "tasks/text-select.hh"
#include "tools/tool-actions.hh"
#include "text/auto-complete.hh"
#include "text/char-constants.hh"
#include "text/formatting.hh"
#include "tools/tool-contexts.hh"
#include "util-wx/key-codes.hh"
#include "util/command-util.hh"
#include "util/pos-info.hh"
#include "util/undo-redo.hh"

namespace faint{

class TextChange{
public:
  TextChange(const std::function<void()>& doFunc,
    const std::function<void()>& undoFunc,
    const utf8_string& name)
    : m_do(doFunc),
      m_undo(undoFunc),
      m_name(name)
  {}

  void Do(){
    m_do();
  }

  const utf8_string& GetName() const{
    return m_name;
  }

  void Undo(){
    m_undo();
  }
private:
  std::function<void()> m_do;
  std::function<void()> m_undo;
  utf8_string m_name;
};


class TextCommand : public Command{
public:
  TextCommand(const TextChange& change)
    : Command(CommandType::OBJECT),
      m_change(change)
  {}

  void Do(CommandContext&) override{
    m_change.Do();
  }

  void Undo(CommandContext&) override{
    m_change.Undo();
  }

  utf8_string Name() const override{
    return m_change.GetName();
  }

  TextCommand& operator=(const TextCommand&) = delete;

private:
  TextChange m_change;
};

inline bool is_exit_key(const KeyPress& key){
  return key.Is(Ctrl, key::enter) ||
    key.Is(key::esc);
}

inline bool outside(Object* obj, const PosInfo& info){
  return !bounding_rect(obj->GetTri()).Contains(info.pos);
}

inline bool right_click(const PosInfo& info){
  return info.modifiers.RightMouse();
}

static Optional<TextChange> handle_command_key(const KeyPress& key,
  ObjText* obj)
{
  auto toggler = [obj](const BoolSetting& setting, const utf8_string& name){
    auto setter = [obj](const auto& setting, const auto& value){
      return [obj, setting, value](){
        return obj->Set(setting, value);
      };
    };

    auto value = obj->GetSettings().Get(setting);
    return TextChange(setter(setting, !value), setter(setting, value),
      value ? "Clear " + name : "Set " + name);
  };

  if (key.Is(Ctrl, key::B)){
    return option(toggler(ts_FontBold, "bold"));
  }
  else if (key.Is(Ctrl, key::I)){
    return option(toggler(ts_FontItalic, "italic"));
  }
  return no_option();
}

static bool handle_completion(AutoCompleteState& autoComplete, TextBuffer& text){
  // Find the preceeding backslash or character halting auto-completion
  const size_t bs = text.prev_any_of(utf8_string(chars::backslash) +
    utf8_string(chars::space) +
    utf8_string(chars::comma) +
    utf8_string(chars::eol));

  if (bs == utf8_string::npos || text.at(bs) != chars::backslash){
    return false;
  }

  auto completion = autoComplete.Empty() ?
    autoComplete.Complete(text.get().substr(bs, text.caret() - bs)) :
    autoComplete.Next();
  text.select(CaretRange(bs, text.caret()));
  text.insert(completion);
  return true;
}

static void select_word_at_pos(ObjText* textObject, const Point& pos){
  size_t caret = textObject->CaretPos(pos);
  TextBuffer& textBuffer = textObject->GetTextBuffer();
  textBuffer.select(word_boundaries(caret, textBuffer));
}

static AutoComplete& text_auto_complete(){
  static AutoComplete ac(expression_names());
  return ac;
}

class EditText : public Task,
                 public TextContext,
                 public SelectionContext,
                 public HistoryContext{
public:
  EditText(const Rect& r,
    const utf8_string& str,
    Settings& settings,
    ToolActions& actions)
    : m_actions(actions),
      m_active(false),
      m_autoComplete(text_auto_complete()),
      m_newTextObject(true),
      m_settings(settings),
      m_textObject(new ObjText(tri_from_rect(r), str, settings))
  {}

  EditText(ObjText* obj, Settings& s, ToolActions& actions)
    : m_actions(actions),
      m_active(false),
      m_autoComplete(text_auto_complete()),
      m_newTextObject(false),
      m_oldText(obj->GetTextBuffer().get()),
      m_settings(s),
      m_textObject(obj)
  {
    m_settings = m_textObject->GetSettings();
  }

  ~EditText(){
    if (m_active){
      EndEntry();
    }
  }

  void Activate() override{
    assert(!m_active);
    m_textObject->SetActive(true);
    m_textObject->SetEdited(true);
    TextBuffer& buf(m_textObject->GetTextBuffer());
    buf.caret(buf.size());
    m_actions.BeginTextEntry();
    m_active = true;
  }

  bool AcceptsPastedText() const override{
    return true;
  }

  bool AllowsGlobalRedo() const override{
    return !m_active;
  }

  bool CanUndo() const override{
    return m_active && m_states.CanUndo();
  }

  bool CanRedo() const override{
    return m_active && m_states.CanRedo();
  }

  utf8_string GetRedoName() const override{
    return m_states.PeekRedo().GetName();
  }

  utf8_string GetUndoName() const override{
    return m_states.PeekUndo().GetName();
  }

  bool RefreshOnMouseOut() const override{
    return false;
  }

  void Redo() override{
    m_states.Redo().Do();
  }

  void Undo() override{
    m_states.Undo().Undo(); // Fixme: Bizarre: Undo just moves between lists.
  }

  TaskResult Char(const KeyInfo& info) override{
    auto handleKeyPress = [&](){
      m_autoComplete.Forget();
      return handle_key_press(info.key, m_textObject->GetTextBuffer()) ?
        TaskResult::DRAW : TaskResult::NONE;
    };

    if (is_exit_key(info.key)){
      return Commit(info.layerType);
    }
    else if (info.key.Is(key::tab) && m_settings.Get(ts_ParseExpressions)){
      return handle_completion(m_autoComplete, m_textObject->GetTextBuffer()) ?
        TaskResult::DRAW :
        handleKeyPress();
    }

    return handle_command_key(info.key, m_textObject).Visit(
      [&](TextChange& cmd){
        m_autoComplete.Forget();
        // Fixme: Need to support undo
        cmd.Do();
        m_states.Did(cmd);
        return TaskResult::DRAW;
      },
      handleKeyPress);
  }

  Optional<utf8_string> CopyText() const override{
    TextBuffer& buffer(m_textObject->GetTextBuffer());
    return option(buffer.get_selection());
  }

  Optional<utf8_string> CutText() override{
    TextBuffer& buffer(m_textObject->GetTextBuffer());
    Optional<utf8_string> s = buffer.get_selection();
    buffer.del();
    return s;
  }

  bool Delete() override{
    m_textObject->GetTextBuffer().del();
    return true;
  }

  bool Deselect() override{
    m_textObject->GetTextBuffer().select_none();
    return true;
  }

  TaskResult DoubleClick(const PosInfo& info) override{
    select_word_at_pos(m_textObject, info.pos);
    return TaskResult::DRAW;
  }

  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override{
    overlays.Corners(m_textObject->GetTri());

    if (m_newTextObject){
      m_textObject->Draw(dc, get_expression_context(info));
    }
    if (!m_textObject->HasSelectedRange()){
      overlays.Caret(m_textObject->GetCaret());
    }
  }

  bool DrawBeforeZoom(Layer layer) const override{
    // This is only relevant when first editing raster text or creating
    // a new text object - when editing an existing object, the text
    // object draws itself, independently of the task.
    return layer == Layer::RASTER;
  }

  bool EatsSettings() const override{
    return false; // Fixme: Actually... dunno
  }

  Command* GetCommand() override{
    return m_command.Take();
  }

  Cursor GetCursor(const PosInfo& info) const override{
    if (m_textObject->GetTri().Contains(info.pos)){
      return Cursor::CARET;
    }
    return Cursor::ARROW;
  }

  Task* GetNewTask() override{
    return m_newTask.Take();
  }

  IntRect GetRefreshRect(const RefreshInfo&) const override{
    return m_textObject->GetRefreshRect();
  }

  TaskResult MouseDown(const PosInfo& info) override{
    assert(m_textObject != nullptr);
    if (right_click(info) || outside(m_textObject, info)){
      return Commit(info.layerType);
    }
    m_newTask.Set(select_text_task(m_textObject, m_newTextObject, info.pos));
    return TaskResult::PUSH;
  }

  TaskResult MouseUp(const PosInfo&) override{
    return TaskResult::NONE;
  }

  TaskResult MouseMove(const PosInfo& info) override{
    info.status.SetMainText("Use Ctrl+Enter to stop editing");
    info.status.SetText(str_floor(info.pos));
    return TaskResult::NONE;
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return Optional<const faint::HistoryContext&>(*this);
  }

  void Paste(const utf8_string& str) override{
    m_textObject->GetTextBuffer().insert(str);
  }

  TaskResult Preempt(const PosInfo& info) override{
    return Commit(info.layerType);
  }

  bool SelectAll() override{
    TextBuffer& text = m_textObject->GetTextBuffer();
    text.select(text.all());
    return true;
  }

  void SelectionChange() override{
  }

  Optional<const faint::SelectionContext&> SelectionContext() const override{
    return Optional<const faint::SelectionContext&>(*this);
  }

  Optional<const faint::TextContext&> TextContext() const override{
    return Optional<const faint::TextContext&>(*this);
  }

  void UpdateSettings() override{
    m_textObject->UpdateSettings(m_settings);
  }

  void SetLayer(Layer) override{
  }

  EditText& operator=(const EditText&) = delete;

private:
  void EndEntry(){
    assert(m_active);

    m_actions.EndTextEntry();
    if (m_textObject != nullptr){
      m_textObject->SetActive(false);
      m_textObject->SetEdited(false);
    }
    m_active = false;
  }

  TaskResult Commit(Layer layerType){
    if (m_active){
      EndEntry();
    }
    if (m_newTextObject){
      m_command.Set(add_or_draw(m_textObject, layerType));
      return TaskResult::COMMIT_AND_CHANGE;
    }

    const utf8_string& newText(m_textObject->GetTextBuffer().get());
    std::deque<Command*> cmds;
    while (m_states.CanUndo()){
      // Fixme: Dificult to understand (first undo just moves between lists)
      TextChange& change = m_states.Undo();
      change.Undo();
      cmds.push_front(new TextCommand(change));
    }

    if (newText != m_oldText){
      // Fixme: Do all changes, also text edits, via TextChange instead
      cmds.push_back(text_entry_command(m_textObject,
        New(newText),
        Old(m_oldText)));
    }

    if (cmds.empty()){
      // No changes - create no command.
      return TaskResult::CHANGE;
    }
    else {
      m_command.Set(cmds.size() == 1 ?
        cmds.back() :
        command_bunch(CommandType::OBJECT, bunch_name("Modify Text"), cmds));
      return TaskResult::COMMIT_AND_CHANGE;
    }
  }

  ToolActions& m_actions;
  bool m_active;
  AutoCompleteState m_autoComplete;
  PendingCommand m_command;
  PendingTask m_newTask;
  bool m_newTextObject;
  utf8_string m_oldText;
  Settings& m_settings;
  UndoRedo<TextChange> m_states;
  ObjText* m_textObject;
};

Task* edit_text_task(const Rect& r,
  const utf8_string& str,
  Settings& settings,
  ToolActions& actions)
{
  return new EditText(r, str, settings, actions);
}

Task* edit_text_task(ObjText* obj, Settings& settings, ToolActions& actions){
  return new EditText(obj, settings, actions);
}

} // namespace
