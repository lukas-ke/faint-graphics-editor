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

#include <fstream>
#include <map>
#include "wx/toolbar.h"
#include "wx/frame.h"
#include "wx/html/htmlwin.h"
#include "wx/splitter.h"
#include "wx/treectrl.h"
#include "app/resource-id.hh"
#include "gui/art.hh"
#include "gui/help-frame.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/make-event.hh"
#include "util-wx/slice-wx.hh"
#include "util/distinct.hh"
#include "util/optional.hh"

namespace faint{

MAKE_FAINT_COMMAND_EVENT(FAINT_CLOSE_HELP);
MAKE_FAINT_COMMAND_EVENT(FAINT_MAXIMIZE_HELP);
MAKE_FAINT_COMMAND_EVENT(FAINT_BACK_HELP);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_END);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_HOME);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_LINE_UP);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_LINE_DOWN);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_PAGE_DOWN);
MAKE_FAINT_COMMAND_EVENT(FAINT_SCROLL_HELP_PAGE_UP);

static bool send_event(wxWindow* window, int eventId){
  wxCommandEvent newEvent(eventId);
  window->GetEventHandler()->ProcessEvent(newEvent);
  return true;
}

static bool is_web_link(const wxHtmlLinkInfo& link){
  auto href = link.GetHref();
  return href.StartsWith("http://") || href.StartsWith("https://");
}

static bool is_help_link(const wxHtmlLinkInfo& link){
  return !is_web_link(link);
}

bool common_help_key(wxWindow* window, const wxKeyEvent& event){
  // Handle key-presses common to windows within the HelpFrame. The
  // window parameter is the window that received the event, and is used
  // for propagating new events to the frame
  if (event.GetKeyCode() == WXK_ESCAPE){
    return send_event(window, FAINT_CLOSE_HELP);
  }
  else if (event.GetKeyCode() == WXK_RETURN && event.AltDown()){
    return send_event(window, FAINT_MAXIMIZE_HELP);
  }
  else if (event.GetKeyCode() == WXK_BACK){
    return send_event(window, FAINT_BACK_HELP);
  }
  else if (event.GetKeyCode() == WXK_SPACE || event.GetKeyCode() == WXK_PAGEDOWN){
    return send_event(window, FAINT_SCROLL_HELP_PAGE_DOWN);
  }
  else if (event.GetKeyCode() == WXK_PAGEUP){
    return send_event(window, FAINT_SCROLL_HELP_PAGE_UP);
  }
  else if (event.GetKeyCode() == WXK_UP && event.ControlDown()){
    return send_event(window, FAINT_SCROLL_HELP_LINE_UP);
  }
  else if (event.GetKeyCode() == WXK_DOWN && event.ControlDown()){
    return send_event(window, FAINT_SCROLL_HELP_LINE_DOWN);
  }
  else if (event.GetKeyCode() == WXK_HOME){
    return send_event(window, FAINT_SCROLL_HELP_HOME);
  }
  else if (event.GetKeyCode() == WXK_END){
    return send_event(window, FAINT_SCROLL_HELP_END);
  }
  return false;
}

wxSplitterWindow* create_help_splitter(wxWindow* parent){
  auto splitter = make_wx<wxSplitterWindow>(parent);

  // Prevent unsplitting
  splitter->SetMinimumPaneSize(20);

  // When rescaling, grow only the right window (help contents)
  splitter->SetSashGravity(0.0);
  return splitter;
}

class HelpWindow : public wxHtmlWindow{
// The html area for the help-text. Uses the basic wxHtmlWindow to
// avoid heavier dependencies (like wxWebView), this should be enough
// for the help system.
public:
  explicit HelpWindow(wxWindow* parent)
    : wxHtmlWindow(parent,
      wxID_ANY,
      wxDefaultPosition,
      wxDefaultSize,
      wxHW_DEFAULT_STYLE | wxBORDER_THEME)
  {
    bind_fwd(this, wxEVT_KEY_DOWN,
      [this](wxKeyEvent& event){
        bool handled = common_help_key(this, event);
        if (!handled){
          event.Skip();
        }
      });
  }

  void FaintLineDown(){
    wxPoint pos = GetViewStart();
    Scroll(pos.x, pos.y + 1);
  }

  void FaintLineUp(){
    wxPoint pos = GetViewStart();
    Scroll(pos.x, pos.y - 1);
  }

  void FaintPageDown(){
    int steps = GetScrollPageSize(wxVERTICAL);
    wxPoint pos = GetViewStart();
    Scroll(pos.x, pos.y + steps - m_keep);
  }

  void FaintPageUp(){
    int steps = GetScrollPageSize(wxVERTICAL);
    wxPoint pos = GetViewStart();
    Scroll(pos.x, pos.y - steps + m_keep);
  }

  void FaintHome(){
    wxPoint pos = GetViewStart();
    Scroll(pos.x, 0);
  }

  void FaintEnd(){
    wxPoint pos = GetViewStart();
    int x, y;
    GetVirtualSize(&x, &y);
    Scroll(pos.x, y);
  }
private:
  void OnLinkClicked(const wxHtmlLinkInfo& link) override{
    if (is_web_link(link)){
      // External links should open in the default browser, not the
      // help window.
      wxLaunchDefaultBrowser(link.GetHref());
    }
    else{
      wxHtmlWindow::OnLinkClicked(link);
    }
  }

  // (Approximate-) lines to keep from the current page while
  // scrolling a page, to make it easier to follow scrolling with pgdn, pgup
  static const int m_keep = 2;
};

class HelpTree;
using page_filename = Distinct<wxString, HelpTree, 0>;
using page_map_t = std::map<wxTreeItemId, page_filename>;

static size_t content_item_depth(const wxString& s){
  size_t depth = 0;
  for (size_t i = 0; i != s.size(); i++){
    if (s[i] == '>'){
      depth += 1;
    }
    else{
      break;
    }
  }
  return depth;
}

class HelpTree : public wxTreeCtrl{
// The tree-based table of contents for the HelpFrame.
public:
  HelpTree(wxWindow* parent, const wxString& contentsFile) :
    wxTreeCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
      wxTR_DEFAULT_STYLE|wxTR_HIDE_ROOT|wxTR_NO_LINES|wxTR_TWIST_BUTTONS)
  {
    m_root = AddRoot("");
    CreateFromFile(contentsFile);

    bind_fwd(this, wxEVT_KEY_DOWN,
      [this](wxKeyEvent& event){

        bool handled = common_help_key(this, event);
        if (!handled){
          event.Skip();
        }
      });
  }

  wxTreeItemId AddPage(const wxString& title, const page_filename& page){
    return AddChildPage(m_root, title, page);
  }

  wxTreeItemId AddChildPage(const wxTreeItemId& parent,
    const wxString& title,
    const page_filename& page)
  {
    wxTreeItemId id = AppendItem(parent, title);
    m_idToPage.insert({id, page});
    return id;
  }

  page_filename GetPage(const wxTreeItemId& id){
    return m_idToPage.at(id);
  }

  void Next(){
    wxTreeItemId selected = GetSelection();
    if (!selected.IsOk()){
      return;
    }
    wxTreeItemId next = GetNextSibling(selected);
    if (!next.IsOk()){
      return;
    }
    SelectItem(next);
  }

  void Prev(){
    wxTreeItemId selected = GetSelection();
    if (!selected.IsOk()){
      return;
    }
    wxTreeItemId prev = GetPrevSibling(selected);
    if (!prev.IsOk()){
      return;
    }
    SelectItem(prev);
  }

  void SelectPage(const page_filename& name){
    for (const auto& mapItem : m_idToPage){
      if (mapItem.second == name){
        SelectItem(mapItem.first);
      }
    }
  }
private:
  void CreateFromFile(const wxString& contentsFile){
    std::ifstream f(iostream_friendly(contentsFile));
    std::string s;
    std::vector<wxTreeItemId> parents = {m_root};

    while (std::getline(f,s)){
      if (s.empty()){
        break;
      }
      size_t sep = s.find(";");
      assert(sep != wxString::npos);
      wxString name = slice_up_to(s, sep);
      page_filename filename(slice_from(s, sep + 1));
      auto depth = content_item_depth(name);
      name = slice_from(name, depth);

      while (parents.size() > depth + 1){
        parents.pop_back();
      }
      auto nodeId = AddChildPage(parents.back(), name, filename);
      parents.push_back(nodeId);
    }
  }

  page_map_t m_idToPage;
  wxTreeItemId m_root;
};

const int help_toolbar_back=0;
const int help_toolbar_forward=1;

static page_filename parse_page_filename(const wxString& str){
  size_t pos = str.rfind("/");
  if (pos == wxString::npos){
    pos = str.rfind("\\");
  }
  if (pos == wxString::npos){
    return page_filename("");
  }
  return page_filename(slice_from(str, pos + 1));
}

static page_filename link_to_filename(const wxString& str){
  size_t pos = str.rfind("#");
  if (pos == wxString::npos){
    return page_filename(wxString(str));
  }
  return page_filename(slice_up_to(str, pos));
}

class HelpFrame::HelpFrameImpl : public wxFrame {
public:
  HelpFrameImpl(const wxString& rootDir, const Art& art)
    : wxFrame(null_parent(), wxID_ANY, "Faint Help"),
      m_html(nullptr),
      m_tree(nullptr),
      m_rootDir(rootDir),
      m_initialized(false),
      m_updateOnTree(true)
  {
    SetInitialSize(wxSize(800,600));
    wxSplitterWindow* splitter = create_help_splitter(this);
    m_tree = make_dumb<HelpTree>(splitter, m_rootDir + "/contents.dat");
    m_html = make_dumb<HelpWindow>(splitter);
    splitter->SplitVertically(m_tree.get(), m_html.get());
    splitter->SetSashPosition(200);
    auto toolbar = make_wx<wxToolBar>(this, wxID_ANY);
    toolbar->AddTool(help_toolbar_back, "Back", art.Get(Icon::HELP_BACK));
    toolbar->AddTool(help_toolbar_forward, "Forward", art.Get(Icon::HELP_FORWARD));
    toolbar->Realize();
    SetToolBar(toolbar);

    bind(this, EVT_FAINT_BACK_HELP,
      [this](){
        GoBack();
      });

    bind_fwd(this, wxEVT_CLOSE_WINDOW,
      [this](wxCloseEvent& event){
        if (event.CanVeto()){
          // Hide instead of close if possible
          event.Veto();
          Hide();
        }
        else{
          Destroy();
        }
      });

    bind(this, EVT_FAINT_CLOSE_HELP,
      // Handles custom EVT_CLOSE_HELP from contained windows.
      [this](){
        // Non-forcing close
        Close();
      });

    bind(this, EVT_FAINT_MAXIMIZE_HELP,
      [this](){
        Maximize(!IsMaximized());
      });

    bind_fwd(this, wxEVT_HTML_LINK_CLICKED,
      [this](wxHtmlLinkEvent& evt){
        m_updateOnTree = false;
        wxHtmlLinkInfo link(evt.GetLinkInfo());
        assert(is_help_link(link));
        m_tree->SelectPage(link_to_filename(link.GetHref()));
        evt.Skip();
        m_updateOnTree = true;
      });

    bind(this, EVT_FAINT_SCROLL_HELP_END,
      [this](){
        m_html->FaintEnd();
      });

    bind(this, EVT_FAINT_SCROLL_HELP_HOME,
      [this](){
        m_html->FaintHome();
      });

    bind(this, EVT_FAINT_SCROLL_HELP_LINE_DOWN,
      [this](){
        if (m_tree->HasFocus()){
          m_html->FaintLineDown();
        }
        else{
          m_tree->Next();
        }
      });

    bind(this, EVT_FAINT_SCROLL_HELP_LINE_UP,
      [this](){
        if (m_tree->HasFocus()){
          m_html->FaintLineUp();
        }
        else{
          m_tree->Prev();
        }
      });

    bind(this, EVT_FAINT_SCROLL_HELP_PAGE_DOWN,
      [this](){
        m_html->FaintPageDown();
      });

    bind(this, EVT_FAINT_SCROLL_HELP_PAGE_UP,
      [this](){
        m_html->FaintPageUp();
      });

    bind_fwd(this, wxEVT_TOOL,
      [this](wxCommandEvent& evt){
        // Forward or backward toolbar-buttons.
        if (evt.GetId() == help_toolbar_back){
          GoBack();
        }
        else if (evt.GetId() == help_toolbar_forward){
          GoForward();
        }
      });

    bind_fwd(this, wxEVT_TREE_SEL_CHANGED,
      [this](wxTreeEvent& evt){
        evt.Skip();
        wxTreeItemId newItem = evt.GetItem();
        wxTreeItemId old = evt.GetOldItem();
        if (old.IsOk()){
          m_tree->SetItemBold(evt.GetOldItem(), false);
        }
        m_tree->SetItemBold(newItem);
        if (m_updateOnTree){
          page_filename page(m_tree->GetPage(evt.GetItem()));
          wxFileName htmlFile(m_rootDir + "/" + page.Get());
          m_html->LoadFile(htmlFile);
        }
      });
  }

  bool FaintHasFocus(){
    return HasFocus() || m_html->HasFocus() || m_tree->HasFocus();
  }

  void FaintShow(){
    if (!m_initialized){
      m_html->LoadFile(wxFileName(m_rootDir + "/main.html"));
      m_initialized = true;
    }
    Show();
  }
private:
  void GoBack(){
    m_html->HistoryBack();
    UpdateTreeSelection();
  }
  void GoForward(){
    m_html->HistoryForward();
    UpdateTreeSelection();
  }

  void UpdateTreeSelection(){
    m_tree->SelectPage(parse_page_filename(m_html->GetOpenedPage()));
  }

  dumb_ptr<HelpWindow> m_html;
  dumb_ptr<HelpTree> m_tree;
  const wxString m_rootDir;
  bool m_initialized;
  bool m_updateOnTree;
};

HelpFrame::HelpFrame(const DirPath& rootDir, const Art& art)
  : m_impl(make_dumb<HelpFrameImpl>(to_wx(rootDir.Str()), art))
{
  restore_persisted_state(m_impl.get(), storage_name("HelpFrame"));
}

HelpFrame::~HelpFrame(){
  if (m_impl != nullptr){
    Close();
  }
 }

void HelpFrame::Close(){
  m_impl->Close(true);
  m_impl.reset(nullptr);
}

void HelpFrame::Hide(){
  m_impl->Hide();
}

bool HelpFrame::HasFocus() const{
  return m_impl->FaintHasFocus();
}

bool HelpFrame::IsHidden() const{
  return !IsShown();
}

bool HelpFrame::IsIconized() const{
  return m_impl->IsIconized();
}

bool HelpFrame::IsShown() const{
  return m_impl->IsShown();
}

void HelpFrame::Raise(){
  m_impl->Raise();
}

void HelpFrame::Restore(){
  m_impl->Restore();
}

void HelpFrame::SetIcons(const wxIcon& icon16, const wxIcon& icon32){
  m_impl->SetIcons(bundle_icons(icon16, icon32));
}

void HelpFrame::Show(){
  m_impl->FaintShow();
}

} // namespace
