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

#include <algorithm> // std::min
#include <vector>
#include "wx/textctrl.h"
#include "bitmap/color.hh"
#include "gui/interpreter-ctrl.hh"
#include "text/auto-complete.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-auto-complete.hh"
#include "util-wx/key-codes.hh"

namespace faint{

const wxEventType PYTHON_COMMAND = wxNewEventType();
const wxEventTypeTag<wxCommandEvent> EVT_PYTHON_COMMAND(PYTHON_COMMAND);

// PythonKeyEvent
const wxEventType FAINT_PYTHON_KEY = wxNewEventType();
PythonKeyEvent::PythonKeyEvent(const KeyPress& keyPress)
  : wxCommandEvent(FAINT_PYTHON_KEY, -1),
    m_key(keyPress)
{}

wxEvent* PythonKeyEvent::Clone() const{
  return new PythonKeyEvent(*this);
}

const KeyPress& PythonKeyEvent::GetKey() const{
  return m_key;
}

// Tag for binding
const wxEventTypeTag<PythonKeyEvent> EVT_FAINT_PYTHON_KEY(FAINT_PYTHON_KEY);

static const wxString brace_open = "({[";
static const wxString brace_close = ")}]";

static bool is_closing_brace(const wxChar& ch){
  return brace_close.find(ch) != wxString::npos;
}

static bool is_opening_brace(const wxChar& ch){
  return brace_open.find(ch) != wxString::npos;
}

static wxChar opposite_brace(const wxChar& ch){
  size_t pos = brace_open.find(ch);
  if (pos != wxString::npos){
    return brace_close[pos];
  }
  pos = brace_close.find(ch);
  if (pos != wxString::npos){
    return brace_open[pos];
  }
  assert(false);
  return '\0';
}

static int count_indent(const wxString& str){
  for (size_t i = 0; i != str.size(); i++){
    if (str[i] != ' '){
      return resigned(i);
    }
  }
  return resigned(str.size());
}

const wxEventType FAINT_PENDING_CHAR = wxNewEventType();

class PendingCharEvent : public wxCommandEvent{
public:
  PendingCharEvent(wxChar c)
    : wxCommandEvent(FAINT_PENDING_CHAR, -1),
      m_char(c)
  {}
  wxEvent* Clone() const override{
    return new PendingCharEvent(m_char);
  }

  wxChar GetChar() const{
    return m_char;
  }
private:
  wxChar m_char;
};

// Tag for binding
const wxEventTypeTag<PendingCharEvent> EVT_FAINT_PENDING_CHAR(FAINT_PENDING_CHAR);

static bool in_quote(const wxString& text){
  int quotes = 0;
  for (size_t i = 0; i != text.size(); i++){
    if (text[i] == wxChar('\"') && (i == 0 || text[i-1] != wxChar('\\'))){
      quotes++;
    }
  }
  return quotes % 2 != 0;
}

static void kill_line(wxTextCtrl& text){
  long x, y;
  text.PositionToXY(text.GetInsertionPoint(), &x, &y);
  long len = text.GetLineLength(y);
  text.Remove(text.GetInsertionPoint(), text.GetInsertionPoint() + len - x);
}

static wxFont interpreter_font(){
  return wxFont(wxFontInfo(10).Family(wxFONTFAMILY_TELETYPE));
}

size_t find_open_brace(const wxString& str, size_t closePos){
  wxChar closeBrace = str[closePos];
  assert(is_closing_brace(closeBrace));
  int num = 0;
  for (size_t i = 0; i <= closePos; i++){
    wxChar c = str[closePos - i];
    if (is_closing_brace(c)){
      num++;
    }
    else if (is_opening_brace(c)){
      num--;
    }
    if (num == 0){
      return closePos - i;
    }
  }
  return wxString::npos;
}

static wxTextAttr matched_brace_attr(){
  wxColour fg(0,0,0);
  wxColour bg(230,209,0);
  wxTextAttr attr_matched_brace(fg, bg,
    interpreter_font());
  attr_matched_brace.SetFontWeight(wxFONTWEIGHT_BOLD);
  //attr_matched_brace.SetFontUnderlined(true);

  return attr_matched_brace;
}

static wxTextAttr unmatched_brace_attr(){
  wxColour fg(255,0,0);
  wxColour bg(wxNullColour);
  wxTextAttr attr_matched_brace(fg, bg,
    interpreter_font());
  attr_matched_brace.SetFontWeight(wxFONTWEIGHT_BOLD);
  return attr_matched_brace;
}

static wxTextAttr mismatched_brace_attr(){
  wxColour fg(0,0,255);
  wxColour bg(255,0,0);
  wxTextAttr attr_matched_brace(fg, bg,
    interpreter_font());
  attr_matched_brace.SetFontWeight(wxFONTWEIGHT_BOLD);
  return attr_matched_brace;

}

class InterpreterImpl : public wxTextCtrl {
public:
  InterpreterImpl(wxWindow* parent, const ColRGB& text, const ColRGB& bg)
    : wxTextCtrl(parent, wxID_ANY, "", wxDefaultPosition, wxDefaultSize,
      wxTE_MULTILINE | wxTE_RICH | wxTE_DONTWRAP),
      m_acDict(),
      m_completion(m_acDict)
  {
    m_fileCompletion = new FileAutoComplete();
    m_inputStart = 0;
    m_getKey = false;
    m_currRowStart = 0;
    m_historyIndex = 0;
    m_indentDepth = 0;

    SetBackgroundColour(to_wx(bg));
    SetDefaultStyle(wxTextAttr(to_wx(text), to_wx(bg),
        interpreter_font()));

    WriteText("Faint Python Interpreter\n");
    SetFocus();
    const auto pos = GetLastPosition();
    SetSelection(pos, pos);

    bind_fwd(this, wxEVT_CHAR,
      [this](wxKeyEvent& event){
        // The char-handler is used for characters (normal text
        // entry). wxWidgets wxEVT_CHAR-handlers receives keycodes
        // with case depending on the shift key, and further modified
        // by ctrl etc.

        int keyCode = event.GetKeyCode();
        m_completion.Forget();

        bool navigation =
          keyCode == WXK_UP ||
          keyCode == WXK_DOWN ||
          keyCode == WXK_RIGHT ||
          keyCode == WXK_LEFT ||
          keyCode == WXK_END ||
          keyCode == WXK_HOME;

        if (navigation){
          event.Skip();
          return;
        }

        if (GetInsertionPoint() < m_inputStart){
          // Don't allow editing in read only regions
          SetInsertionPoint(m_inputStart);
          return;
        }

        else if (keyCode == WXK_RETURN){
          #ifndef __WXMSW__
          AppendText("\n");
          #endif
          PushLine();
          return;
        }
        else {
          // SetDefaultStyle affects WriteText, but it does not affect
          // normal text entry (i.e. what would happen on event.Skip() for
          // the char-event, so I need to do WriteText instead to get the
          // correct font and color.

          // ...However: Using WriteText in a char handler crashes in gtk
          // after some wxWidgets change (maybe r73756), so I need to
          // postpone the writing.
          QueueEvent(new PendingCharEvent(wxChar(keyCode)));
        }
      });

    bind_fwd(this, wxEVT_KEY_DOWN,
      [this](wxKeyEvent& event){
        // The key-down handler is used for commands (e.g. ctrl+c
        // etc.), as opposed to the wxEVT_CHAR handler.
        //
        // Note: wxWidgets EVT_KEY_DOWN-handlers receive the keycode
        // (e.g. for 'a') as an uppercase letter ('A') - modifiers can
        // be retrieved separately from the KeyEvent
        if (!event.ShiftDown()){
          // Remove any previous style (causes problems with selection
          // if shift is held)
          DefaultStyleLine();
        }

        int keyCode = event.GetKeyCode();
        if (m_getKey){
          // Retrieve the keypress which the interpreter has requested
          // from the user (e.g. for a key bind or a yes/no query)

          if (key::modifier(keyCode)){
            // Modifier keys are relevant, but should not complete the
            // key-query.
            return;
          }

          m_getKey = false;
          GotKey(KeyPress(key_modifiers(event), Key(keyCode)));
          AppendText("\n");
          NewPrompt();
          return;
        }

        if (keyCode == WXK_RETURN){
          if (event.ControlDown()){
            // Ignore enter when control held as this outputs some
            // garbage character. Attempts to change this with
            // Set[Raw]ControlDown didn't help much.
            return;
          }

          // Move to end of line on enter before inserting line break
          // and comitting
          wxString lineText = GetRange(BeginningOfLine(), GetLastPosition());
          if (!lineText.empty()){
            m_history.push_back(lineText);
          }
          m_historyIndex = m_history.size();

          if (GetInsertionPoint() != GetLastPosition()){
            SetInsertionPoint(GetLastPosition());
          }
          event.Skip();
          return;
        }

        if (keyCode == WXK_ESCAPE){
          GetParent()->Close();
          return;
        }

        if (keyCode == WXK_TAB){
          if (in_quote(GetRange(m_inputStart, GetLastPosition()))) {
            PathCompletion(event.ShiftDown());
          }
          else {
            CommandCompletion(event.ShiftDown());
          }
          return;
        }

        if (keyCode != WXK_SHIFT){
          // Forget autocompletion state when a new entry is performed
          // (shift is ignored to support shift+tab, and it does not
          // cause an entry)
          m_fileCompletion->Forget();
          m_completion.Forget();
        }

        if (keyCode == WXK_BACK){
          if (HasSelection()){
            if (SelectionEditable()){
              DeleteSelection();
            }
          }
          else if (GetInsertionPoint() > m_inputStart){
            Remove(GetInsertionPoint() - 1, GetInsertionPoint());
          }
          return;
        }
        else if (keyCode == WXK_DELETE){
          if (HasSelection()){
            if (SelectionEditable()){
              DeleteSelection();
            }
          }
          else {
            // No selection, remove at insertion point
            if (Editable(GetInsertionPoint())) {
              Remove(GetInsertionPoint(), GetInsertionPoint() + 1);
            }
          }
          return;
        }
        else if (keyCode == WXK_HOME){
          const long insertionPoint = GetInsertionPoint();
          SetInsertionPoint(m_inputStart);
          if (event.ShiftDown()){
            SetSelection(insertionPoint, m_inputStart);
          }
          return;
        }
        else if (event.AltDown() || (wxMOD_ALTGR == (event.GetModifiers() & wxMOD_ALTGR))){
          if (keyCode == wxChar('K')){
            kill_line(*this);
            return;
          }
          else if (keyCode == wxChar('A')){
            ToBeginningOfEntry();
            return;
          }
          else if (keyCode == wxChar('E')){
            ToEndOfLine();
            return;
          }
          else if (keyCode == wxChar('F')){
            ForwardWord();
            return;
          }
          else if (keyCode == wxChar('B')){
            BackwardWord();
            return;
          }
          else {
            event.Skip();
          }
        }
        else if (event.ControlDown()){
          if (keyCode == wxChar('F')){
            ForwardChar();
          }
          else if (keyCode == wxChar('K')){
            kill_line(*this);
          }
          else if (keyCode == wxChar('A')){
            ToBeginningOfEntry();
          }
          else if (keyCode == wxChar('E')){
            ToEndOfLine();
          }
          else if (keyCode == wxChar('B')){
            BackwardChar();
          }
          else if (keyCode == wxChar('C')){
            Copy();
          }
          else if (keyCode == wxChar('V')){
            if (SelectionEditable()){
              FaintPaste();
              return;
            }
          }
          else if (keyCode == wxChar('X')){
            if (SelectionEditable()){
              Cut();
            }
          }
          else if (keyCode == WXK_UP ||keyCode == WXK_DOWN){
            // Up and down navigates the command history

            if (m_history.empty() || !Editable(GetInsertionPoint())){
              return;
            }

            if (keyCode == WXK_DOWN){
              m_historyIndex = std::min(m_historyIndex + 1, m_history.size());
              if (m_historyIndex == m_history.size()){
                return;
              }
            }
            else if (keyCode == WXK_UP){
              if (m_historyIndex == 0){
                return;
              }
              m_historyIndex -= 1;
            }

            ToBeginningOfLine();
            kill_line(*this);
            WriteText(m_history[m_historyIndex]);
          }
          return;
        }
        else if (keyCode == WXK_LEFT){
          if (event.ShiftDown()){
            event.Skip();
          }
          else{
            BackwardChar();
          }
          return;
        }
        else if (keyCode == WXK_RIGHT){
          if (event.ShiftDown()){
            event.Skip();
          }
          else{
            ForwardChar();
          }
          return;
        }

        // Allow the key event to reach the character handler
        event.Skip();
      });

    bind_fwd(this, EVT_FAINT_PENDING_CHAR,
      [this](PendingCharEvent& event){
        WriteText(wxString(event.GetChar()));
        StyleLine();
      });
  }

  ~InterpreterImpl(){
    delete m_fileCompletion;
  }

  void AddNames(const std::vector<utf8_string>& names){
    for (const auto& n : names){
      m_acDict.add(n);
    }
  }

  void AddText(const wxString& text){
    SetInsertionPoint(GetLastPosition());
    AppendText("\n");
    AppendText(text);
  }

  void GetKey(){
    m_getKey = true;
  }

  void NewContinuation(){
    const auto lastPos = GetLastPosition();
    SetInsertionPoint(lastPos);
    AppendText("... ");
    m_inputStart = GetLastPosition();
    AppendText(wxString(' ', to_size_t(m_indentDepth)));
  }

  void NewPrompt(){
    const auto lastPos = GetLastPosition();
    SetInsertionPoint(lastPos);

    AppendText(m_getKey ? "[ press key ]" : ">>> ");
    m_inputStart = GetLastPosition();
    m_currRowStart = m_inputStart;
    m_indentDepth = 0;
  }

  void SetTextColor(const ColRGB& c){
    SetDefaultStyle(wxTextAttr(to_wx(c),
        GetBackgroundColour(),
        interpreter_font()));
  }

  bool CanPaste() const override{
    return SelectionEditable() && wxTextCtrl::CanPaste();
  }

  bool CanCut() const override{
    return SelectionEditable() && wxTextCtrl::CanCut();
  }

  bool CanRedo() const override{
    return false;
  }
  bool CanUndo() const override{
    return false;
  }
  void Redo() override{
    // Do nothing
  }
  void Undo() override{
    // To avoid erasing prompt markers, do nothing
  }

  void WriteText(const wxString& text) override{
    DefaultStyleLine();
    wxTextCtrl::WriteText(text);
  }

  void DefaultStyleLine(){
    SetStyle(BeginningOfLine(), EndOfLine(), GetDefaultStyle());
  }

  void StyleLine(){
    const auto pos = GetInsertionPoint();    
    if (pos == 0 || ReadOnly(pos -1)){
      return;
    }

    wxString prevChar = GetRange(pos - 1, pos);
    if (is_closing_brace(prevChar[0])){
      wxString line = GetRange(m_inputStart, pos);
      // Fixme: Check cast
      size_t matched = find_open_brace(line, 
        static_cast<size_t>(pos - m_inputStart - 1));
      if (matched != wxString::npos){
        wxTextAttr style = line[matched] == opposite_brace(prevChar[0]) ?
          matched_brace_attr() :
          mismatched_brace_attr();
        SetStyle(BeginningOfLine() + resigned(matched), m_inputStart + resigned(matched) + 1, style);
        SetStyle(pos-1, pos, style);
      }
      else{
        SetStyle(pos-1, pos, unmatched_brace_attr());
      }
    }
  }

private:
  void BackwardChar(){
    SetInsertionPoint(GetInsertionPoint() - 1);
    StyleLine();
  }

  void BackwardWord(){
    long currPos = GetInsertionPoint();
    wxString text = GetRange(m_currRowStart, currPos);
    std::cout << text << std::endl;
    size_t pos = text.rfind(" ");
    std::cout << " ->" << pos << std::endl;
    if (pos != wxString::npos){
      SetInsertionPoint(currPos - resigned((text.size() - pos)));
    }
    else {
      SetInsertionPoint(m_inputStart);
    }
  }

  long BeginningOfEntry() const{
    long lineStart = BeginningOfLine();
    wxString text = GetRange(lineStart, GetLastPosition());
    size_t i = 0;
    for (; i != text.size(); i++){
      if (text[i] != ' '){
        return lineStart + resigned(i);
      }
    }
    return lineStart;
  }

  long BeginningOfLine() const{
    long curr = GetInsertionPoint();
    long x, y;
    PositionToXY(curr, &x, &y);
    return(curr - x + 4);
  }

  void CommandCompletion(bool shiftHeld){
    // Python command completion.
    // Search from the end...
    auto fromPos = GetLastPosition();
    if (m_completion.Has()){
      // ...unless we've completed before, if so
      // search from the completion-start
      fromPos -= m_completion.Get().size();
    }

    wxString text = GetRange(m_inputStart, fromPos);
    // Begin completing from the start of the identifier preceding the caret
    size_t pos = text.find_last_of(" ([{.");
    if (pos != wxString::npos){
      text = text.substr(pos + 1, text.size() - pos);
    }
    else {
      pos = 0;
    }

    wxString adjust = m_completion.Has()?
      (shiftHeld ?
        to_wx(m_completion.Prev()) :
        to_wx(m_completion.Next())) :
      to_wx(m_completion.Complete(to_faint(text)));
    Remove(m_inputStart + resigned((pos == 0 ? 0 : pos + 1)), GetLastPosition());
    AppendText(adjust);
  }

  void DeleteSelection(){
    long start, end;
    GetSelection(&start,&end);
    Remove(start, end);
  }

  bool Editable(long pos) const{
    return !ReadOnly(pos);
  }

  long EndOfLine() const{
    long x, y;
    long curr = GetInsertionPoint();
    PositionToXY(curr, &x, &y);
    long len = GetLineLength(y);
    return curr + len - x;
  }

  void FaintPaste(){
    wxString str(get_clipboard_text());
    std::vector<wxString> lines(wx_split_lines(str));
    if (lines.empty()){
      return;
    }
    else if (lines.size() == 1){
      WriteText(lines[0]);
    }
    else{
      for (size_t i = 0; i != lines.size(); i++){
        WriteText(lines[i] + "\n");
        if (i != lines.size() - 1){
          NewContinuation();
        }
      }
      wxCommandEvent event (EVT_PYTHON_COMMAND, GetId());
      event.SetString(str + "\n");
      GetEventHandler()->ProcessEvent(event);
    }
  }

  void ForwardChar(){
    SetInsertionPoint(GetInsertionPoint() + 1);
    StyleLine();
  }

  void ForwardWord(){
    long currPos = GetInsertionPoint();
    wxString text = GetRange(currPos, GetLastPosition());
    size_t spacePos = text.find(" ");
    if (spacePos != wxString::npos){
      SetInsertionPoint(currPos + resigned(spacePos) + 1);
    }
    else {
      SetInsertionPoint(GetLastPosition());
    }
  }

  void GotKey(const KeyPress& key){
    PythonKeyEvent event(key);
    GetEventHandler()->ProcessEvent(event);
  }

  bool HasSelection() const{
    long start, end;
    GetSelection(&start, &end);
    return start != end;
  }

  void PathCompletion(bool shiftDown){
    wxString text = GetRange(m_inputStart, GetLastPosition());
    size_t pos = text.rfind("\"");
    if (pos != wxString::npos){
      text = text.substr(pos + 1, text.size() - pos);
    }
    else {
      pos = 0;
    }

    wxString adjust = m_fileCompletion->Has() ?
      (shiftDown ? m_fileCompletion->Previous() : m_fileCompletion->Next()) :
      m_fileCompletion->Complete(std::string(text));

    Remove(m_inputStart + resigned(pos) + 1, GetLastPosition());
    AppendText(adjust);
  }

  void PushLine(){
    wxCommandEvent event(EVT_PYTHON_COMMAND, GetId());
    wxString text = GetRange(m_inputStart, GetLastPosition()).Strip();
    if (!text.empty()){
      if (text[text.size() - 1] == ':'){
        m_indentDepth = count_indent(text) + 2;
      }
      else{
        m_indentDepth = count_indent(text);
      }
    }
    event.SetString(text.Strip());
    GetEventHandler()->ProcessEvent(event);
  }

  bool ReadOnly(long pos) const{
    return pos < m_inputStart;
  }

  bool SelectionEditable() const{
    long start, end;
    GetSelection(&start, &end);
    if (ReadOnly(start) || ReadOnly(end)){
      return false;
    }
    return true;
  }

  void ToBeginningOfEntry(){
    long entryStart = BeginningOfEntry();
    long lineStart = BeginningOfLine();
    SetInsertionPoint(entryStart == GetInsertionPoint() ?
      lineStart : entryStart);
  }

  void ToBeginningOfLine(){
    SetInsertionPoint(BeginningOfLine());
  }

  void ToEndOfLine(){
    SetInsertionPoint(EndOfLine());
  }

  wxTextPos m_inputStart;
  wxTextPos m_currRowStart;
  bool m_getKey;
  faint::AutoComplete m_acDict;
  AutoCompleteState m_completion;
  FileAutoComplete* m_fileCompletion;
  std::vector<wxString> m_history;
  size_t m_historyIndex;
  int m_indentDepth;
};

InterpreterCtrl::InterpreterCtrl(wxWindow* parent)
  : m_bgColor(255,249,189),
    m_textColor(0,0,0)
{
  m_impl = new InterpreterImpl(parent, m_textColor, m_bgColor);
}

void InterpreterCtrl::AddNames(const std::vector<utf8_string>& names){
  m_impl->AddNames(names);
}

void InterpreterCtrl::AddText(const wxString& str){
  m_impl->AddText(str);
}

void InterpreterCtrl::AppendText(const wxString& str){
  m_impl->AppendText(str);
}

void InterpreterCtrl::GetKey(){
  m_impl->GetKey();
}

bool InterpreterCtrl::HasFocus() const{
  return m_impl->HasFocus();
}

void InterpreterCtrl::NewContinuation(){
  m_impl->NewContinuation();
}

void InterpreterCtrl::NewPrompt(){
  m_impl->NewPrompt();
}

void InterpreterCtrl::SetBackgroundColor(const ColRGB& c){
  m_bgColor = c;
  m_impl->SetBackgroundColour(to_wx(c));

  // Refresh the text background too
  m_impl->SetDefaultStyle(wxTextAttr(to_wx(m_textColor), to_wx(c),
      interpreter_font()));
}

void InterpreterCtrl::SetTextColor(const ColRGB& c){
  m_textColor = c;
  m_impl->SetTextColor(c);
}

} // namespace
