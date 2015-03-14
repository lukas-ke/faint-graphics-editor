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

#include "wx/wx.h"
#include "wx/filename.h"
#include "wx/treebook.h"
#include "wx/stdpaths.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "commands/bitmap-cmd.hh"
#include "gui/art-container.hh"
#include "gui/command-window.hh"
#include "gui/dialog-context.hh"
#include "generated/resource/load-resources.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"
#include "util/status-interface.hh"

void add_gui_tests(wxBookCtrlBase*,
  faint::StatusInterface&,
  faint::DialogContext&);


class GuiTestStatusInterface : public faint::StatusInterface{
public:
  void SetStatusBar(wxStatusBar* sb){
    m_sb = sb;
    m_sb->SetFieldsCount(2);
  }

  void SetMainText(const faint::utf8_string& text) override{
    m_sb->SetStatusText(to_wx(text), 0);
  }

  void SetText(const faint::utf8_string& text, int field) override{
    m_sb->SetStatusText(to_wx(text), field + 1);
  }
  void Clear() override{
    for (int i = 0; i != m_sb->GetFieldsCount(); i++){
      m_sb->SetStatusText("", i);
    }
  }

private:
  wxStatusBar* m_sb = nullptr;
};

template<typename T>
class GuiTestWindowFeedbackImpl : public faint::WindowFeedback{
public:
  GuiTestWindowFeedbackImpl(const T& onClose) :
    m_onClose(onClose)
  {}

  void Closed() override{
    m_onClose(nullptr);
  }

  void Closed(faint::BitmapCommand* c) override{
    m_onClose(c);
  }

  faint::Bitmap GetBitmap() override{
    return faint::Bitmap(faint::IntSize(10,10), faint::Color(255,0,255));
  }

  void SetBitmap(const faint::Bitmap&) override{}
  void Reset() override{}
  void UpdateSettings(const faint::Settings&) override{}
  void Reinitialize() override{}

private:
  T m_onClose;
};

template<typename T>
std::unique_ptr<GuiTestWindowFeedbackImpl<T>>
create_window_feedback(const T& onClose){
  return std::make_unique<GuiTestWindowFeedbackImpl<T>>(onClose);
}

class GuiTestDialogContext : public faint::DialogContext{
public:
  GuiTestDialogContext(wxWindow* parent)
    : m_parent(parent)
  {
    m_windowFeedback = std::move(create_window_feedback(
      [this](faint::BitmapCommand* cmd){
        OnClosed(cmd);
      }));
  }
  void Show(std::unique_ptr<faint::CommandWindow>&& w) override{
    assert(w != nullptr);
    m_commandWindow = std::move(w);
    m_commandWindow->Show(*m_parent, *m_windowFeedback);
  }

private:
  void OnClosed(faint::BitmapCommand* cmd){
    m_commandWindow.reset(nullptr);
    delete cmd;
  }
  void BeginModalDialog() override {};
  void EndModalDialog() override {};
  std::unique_ptr<faint::CommandWindow> m_commandWindow;
  std::unique_ptr<faint::WindowFeedback> m_windowFeedback;
  wxWindow* m_parent;
};

class GuiTestFrame: public wxFrame {
public:
  GuiTestFrame(const wxString& title, const wxPoint& pos, const wxSize& size)
    : wxFrame(nullptr, wxID_ANY, title, pos, size),
      m_dialogContext(this)
  {
    wxMenu *menuFile = new wxMenu;
    menuFile->Append(wxID_EXIT);
    wxMenuBar *menuBar = new wxMenuBar;
    menuBar->Append(menuFile, "&File");
    SetMenuBar(menuBar);

    wxStatusBar* statusbar = new wxStatusBar(this, wxID_ANY);
    this->SetStatusBar(statusbar);
    statusbar->SetFieldsCount(2);
    m_statusInterface.SetStatusBar(statusbar);

    faint::bind_menu(this, wxID_EXIT, [&](){Close(true);});

    auto pageList = new wxTreebook(this, wxID_ANY);
    add_gui_tests(pageList, m_statusInterface, m_dialogContext);
  }

private:
  GuiTestDialogContext m_dialogContext;
  GuiTestStatusInterface m_statusInterface;
};

static faint::DirPath gui_test_resource_dir(){
  wxStandardPathsBase& paths = wxStandardPaths::Get();
  wxFileName f(faint::absoluted(paths.GetExecutablePath()).GetPath());
  wxString pathStr(f.GetFullPath());
  pathStr.Replace("\\", "/");
  return faint::DirPath(pathStr).SubDir("../graphics");
}

class GuiTestApp: public wxApp {
public:
  bool OnInit() override{
    wxInitAllImageHandlers();
    m_art.SetRoot(gui_test_resource_dir());
    load_faint_resources(m_art);
    GuiTestFrame *frame = new GuiTestFrame("Faint GUI test",
      wxDefaultPosition,
      wxSize(450, 340));
    frame->Show(true);
    return true;
  }

  faint::ArtContainer m_art;
};

wxIMPLEMENT_APP(GuiTestApp);

namespace faint{

const ArtContainer& get_art_container(){
  // Fixme: Should not need this function in a test.
  return wxGetApp().m_art;
}

} // namespace
