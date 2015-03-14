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

#include "python/py-include.hh" // Early to avoid HAVE_SSIZE_T redefine warning
#include "wx/frame.h"
#include "gui/interpreter-ctrl.hh"
#include "gui/interpreter-frame.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/gui-util.hh"

namespace faint{

static void run_string_interpreter(const utf8_string& str, scoped_ref& module){
  assert(module != nullptr);
  auto dict = borrowed(PyModule_GetDict(module.get()));
  auto pushFunc = borrowed(PyDict_GetItemString(dict.get(), "push"));
  assert(pushFunc != nullptr);
  assert(PyCallable_Check(pushFunc.get()));
  scoped_ref args(Py_BuildValue("(s)",str.c_str()));
  scoped_ref result(PyObject_CallObject(pushFunc.get(), args.get()));
  scoped_ref err(PyErr_Occurred());
  if (err != nullptr){
    PyErr_PrintEx(0);
  }
}

class InterpreterFrameImpl : public wxFrame{
public:
  InterpreterFrameImpl()
    : wxFrame(null_parent(), wxID_ANY, "Faint Python Interpreter")
  {
    m_interpreterCtrl = make_dumb<InterpreterCtrl>(this);
    SetMinSize(wxSize(200,200));
    SetSize(600,400);

    bind_fwd(this, EVT_FAINT_PYTHON_KEY,
      [](PythonKeyEvent& event){
        const KeyPress& key(event.GetKey());
        wxString cmd = wxString::Format("ifaint.bind2(%d,%d)",
          (int)key.GetKeyCode(),
          key.Modifiers().Raw());
        PyRun_SimpleString(cmd.mb_str());
      });

    bind_fwd(this, EVT_PYTHON_COMMAND,
      [&](wxCommandEvent& event){
        run_string_interpreter(to_faint(event.GetString()), GetModule());
      });

    bind_fwd(this, wxEVT_CLOSE_WINDOW,
      [this](wxCloseEvent& event){
        if (event.CanVeto()){
          // Hide, instead of closing the frame if possible
          event.Veto();
          Hide();
        }
        else{
          // .. otherwise destroy it (application exiting).
          Destroy();
        }
      });
  }

  InterpreterCtrl& GetInterpreterCtrl(){
    return *m_interpreterCtrl;
  }

private:
  scoped_ref& GetModule(){
    if (m_ifaint == nullptr){
      m_ifaint.reset(PyImport_ImportModule("ifaint"));
    }
    return m_ifaint;
  }
  dumb_ptr<InterpreterCtrl> m_interpreterCtrl;
  scoped_ref m_ifaint;
};

InterpreterFrame::InterpreterFrame()
  : m_impl(make_dumb<InterpreterFrameImpl>())
{
  restore_persisted_state(m_impl.get(), storage_name("InterpreterFrame"));
}

InterpreterFrame::~InterpreterFrame(){
  if (m_impl != nullptr){
    m_impl->Close();
    m_impl = nullptr;
  }
}

void InterpreterFrame::AddNames(const std::vector<utf8_string>& names){
  m_impl->GetInterpreterCtrl().AddNames(names);
}

void InterpreterFrame::Close(){
  m_impl->Close(true);
  m_impl = nullptr;
}

bool InterpreterFrame::HasFocus() const{
  return m_impl->HasFocus() || m_impl->GetInterpreterCtrl().HasFocus();
}

void InterpreterFrame::Hide(){
  m_impl->Hide();
}

bool InterpreterFrame::IsIconized() const{
  return m_impl->IsIconized();
}

void InterpreterFrame::Raise(){
  m_impl->Raise();
}

void InterpreterFrame::Restore(){
  return m_impl->Restore();
}

void InterpreterFrame::SetBackgroundColor(const ColRGB& c){
  m_impl->GetInterpreterCtrl().SetBackgroundColor(c);
}

void InterpreterFrame::SetIcons(const wxIcon& icon16, const wxIcon& icon32){
  m_impl->SetIcons(bundle_icons(icon16, icon32));
}

void InterpreterFrame::Show(){
  m_impl->Show();
}

void InterpreterFrame::GetKey(){
  m_impl->GetInterpreterCtrl().GetKey();
}

void InterpreterFrame::IntFaintPrint(const utf8_string& s){
  m_impl->GetInterpreterCtrl().AppendText(to_wx(s));
}

bool InterpreterFrame::IsHidden() const{
  return !m_impl->IsShown();
}

bool InterpreterFrame::IsMaximized() const{
  return !m_impl->IsMaximized();
}

void InterpreterFrame::Maximize(bool b){
  m_impl->Maximize(b);
}

void InterpreterFrame::NewContinuation(){
  m_impl->GetInterpreterCtrl().NewContinuation();
}

void InterpreterFrame::NewPrompt(){
  m_impl->GetInterpreterCtrl().NewPrompt();
}

void InterpreterFrame::Print(const utf8_string& s){
  m_impl->GetInterpreterCtrl().AddText(to_wx(s));
}

void InterpreterFrame::SetTextColor(const ColRGB& c){
  m_impl->GetInterpreterCtrl().SetTextColor(c);
}

} // namespace
