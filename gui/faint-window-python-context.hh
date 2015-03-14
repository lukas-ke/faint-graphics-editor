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

#ifndef FAINT_WINDOW_PYTHON_CONTEXT_HH
#define FAINT_WINDOW_PYTHON_CONTEXT_HH
#include <set>
#include "gui/interpreter-frame.hh"
#include "python/python-context.hh"

namespace faint{

class FaintWindowPythonContext : public PythonContext {
public:
  FaintWindowPythonContext(FaintWindow& window, InterpreterFrame& interpreter)
    : m_interpreterFrame(interpreter),
      m_window(window)
  {}

  void Bind(const KeyPress& key, const bind_global& bindGlobal) override{
    if (bindGlobal.Get()){
      m_globalBinds.insert(key);
      m_binds.erase(key);
    }
    else{
      m_binds.insert(key);
      m_globalBinds.erase(key);
    }
  }

  bool Bound(const KeyPress& key) const override{
    return m_binds.find(key) != m_binds.end();
  }

  bool BoundGlobal(const KeyPress& key) const override{
    return m_globalBinds.find(key) != m_globalBinds.end();
  }

  void Continuation() override{
    m_interpreterFrame.NewContinuation();
  }

  void EvalDone() override{
    for (Canvas* canvas : m_unrefreshed){
      if (m_window.Exists(m_canvasIds[canvas])){
        canvas->Refresh();
      }
    }
    m_unrefreshed.clear();

    for (Canvas* canvas : m_commandBundles){
      canvas->CloseUndoBundle(GetCommandName(canvas));
    }
    m_commandBundles.clear();
    m_commandNames.clear();
  }

  utf8_string GetCommandName(const Canvas* canvas) override{
    auto it = m_commandNames.find(canvas);
    return it == m_commandNames.end() ?
      "" : it->second;
  }

  void GetKey() override{
    m_interpreterFrame.GetKey();
  }

  void IntFaintPrint(const utf8_string& s) override{
    m_interpreterFrame.IntFaintPrint(s);
  }

  void NewPrompt() override{
    m_interpreterFrame.NewPrompt();
  }

  void Print(const utf8_string& s) override{
    m_interpreterFrame.Print(s);
  }

  void QueueRefresh(Canvas* canvas) override{
    m_unrefreshed.insert(canvas);
    m_canvasIds[canvas] = canvas->GetId();
  }

  void RunCommand(Canvas* canvas, Command* command) override{
    QueueRefresh(canvas);
    if (std::find(begin(m_commandBundles), end(m_commandBundles), canvas) ==
      m_commandBundles.end())
    {
      m_commandBundles.push_back(canvas);
      canvas->OpenUndoBundle();
    }
    canvas->RunCommand(command);
  }

  void RunCommand(Canvas* canvas, Command* command,
    const FrameId& frameId) override
  {
    QueueRefresh(canvas);
    if (std::find(begin(m_commandBundles), end(m_commandBundles), canvas) ==
      end(m_commandBundles))
    {
      m_commandBundles.push_back(canvas);
      canvas->OpenUndoBundle();
    }
    canvas->RunCommand(command, frameId);
  }

  void SetCommandName(const Canvas* canvas, const utf8_string& name) override{
    m_commandNames[canvas] = name;
  }

  void Unbind(const KeyPress& key) override{
    m_binds.erase(key);
  }

private:
  std::set<KeyPress> m_binds;
  std::map<Canvas*, CanvasId> m_canvasIds;
  std::vector<Canvas*> m_commandBundles;
  std::map<const Canvas*, utf8_string> m_commandNames;
  std::set<KeyPress> m_globalBinds;
  InterpreterFrame& m_interpreterFrame;
  std::set<Canvas*> m_unrefreshed;
  FaintWindow& m_window;
};

} // namespace

#endif
