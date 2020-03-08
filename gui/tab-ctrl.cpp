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

#include "wx/aui/auibook.h"
#include "wx/dnd.h"
#include "gui/canvas-change-event.hh"
#include "gui/canvas-panel.hh"
#include "gui/freezer.hh"
#include "gui/tab-ctrl.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util/generator-adapter.hh"
#include "util/image-props.hh"
#include "util/index-iter.hh"

namespace faint{

const wxEventType FAINT_ACTIVE_CANVAS_CHANGE = wxNewEventType();
const wxEventTypeTag<CanvasChangeEvent> EVT_FAINT_ACTIVE_CANVAS_CHANGE(
  FAINT_ACTIVE_CANVAS_CHANGE);

static wxString get_title(CanvasPanel* canvas){
  const Optional<FilePath> filePath(canvas->GetFilePath());
  wxString title = filePath.IsSet() ?
    to_wx(filePath.Get().StripPath().Str()) :
    get_new_canvas_title();

  if (canvas->IsDirty()){
    title += " *";
  }
  return title;
}

class CanvasFileDropTarget : public wxFileDropTarget {
public:
  explicit CanvasFileDropTarget(AppContext& app)
    : m_app(app)
  {}

  bool OnDropFiles(wxCoord, wxCoord, const wxArrayString& files) override{
    m_app.Load(to_FileList(files));
    return true;
  }
  AppContext& m_app;
};

const auto style = wxAUI_NB_DEFAULT_STYLE | wxWANTS_CHARS;

class TabCtrlImpl : public wxAuiNotebook {
public:
  TabCtrlImpl(wxWindow* parent,
    const Art& art,
    AppContext& app,
    StatusInterface& status)
    : wxAuiNotebook(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style),
      m_app(app),
      m_art(art),
      m_statusInfo(status)
  {
    SetCanFocus(false);
    SetAcceleratorTable(wxNullAcceleratorTable);

    bind_fwd(this, wxEVT_AUINOTEBOOK_PAGE_CLOSE,
      [this, &app](wxAuiNotebookEvent& event){
        event.Veto();
        QueryClose(Index(event.GetSelection()));
      });

    bind(this, wxEVT_AUINOTEBOOK_PAGE_CHANGED,
      [&](){
        const CanvasPanel* canvas = GetCanvasPage(Index(GetSelection()));
        CanvasChangeEvent newEvent(FAINT_ACTIVE_CANVAS_CHANGE,
          canvas->GetCanvasId());
        ProcessEvent(newEvent);
      });

    bind_fwd(this, wxEVT_AUINOTEBOOK_PAGE_CHANGING,
      [this](wxAuiNotebookEvent& event){
        int selection = GetSelection();

        // Selection is -1 if the change was due to page close
        if (GetPageCount() > 1 && selection >= 0){
          assert(selection < resigned(GetPageCount()));
          CanvasPanel* canvas = GetCanvasPage(Index(selection));
          canvas->Preempt(PreemptOption::ALLOW_COMMAND);
        }
        event.Skip();
      });

    events::on_canvas_modified_skip(this,
      [&](CanvasId canvasId){
        RefreshTabName(canvasId);
      });
  }

  void Close(const Index& page, bool force){
    CanvasPanel* canvas = GetCanvasPage(page);
    canvas->Preempt(PreemptOption::ALLOW_COMMAND);
    if (canvas->IsDirty() && !force){
      SaveChoice choice = ask_close_unsaved_tab(this,
        m_app,
        canvas->GetFilePath());
      if (choice == SaveChoice::CANCEL){
        return;
      }

      if (choice == SaveChoice::YES){
        bool saved = m_app.Save(canvas->GetInterface());
        if (!saved){
          return;
        }
      }
    }

    if (GetPageCount() == 1){
      // Remove annyoing flashing appearance on windows when closing
      // last tab. This freeze must only be done when the last tab is
      // closed, because it causes a refresh error if the tabcontrol
      // is split (see issue 86).
      auto freezer = freeze(this);
      DeletePage(to_size_t(page));
    }
    else{
      DeletePage(to_size_t(page));
    }
    if (GetPageCount() == 0){
      // Avoid flicker in the upper-left of the tab-bar from page
      // creation
      auto freezer = freeze(this);

      AddPage(make_wx<CanvasPanel>(this,
          ImageList(ImageProps(m_app.GetDefaultImageInfo())),
          initially_dirty(false),
          CreateFileDropTarget(),
          m_art,
          m_app,
          m_statusInfo),
        get_new_canvas_title(),
        false);
    }
  }

  void QueryClose(const Index& page){
    Close(page, false);
  }

  void CloseActive(){
    if (GetPageCount() == 0){
      return;
    }
    QueryClose(Index(GetSelection()));
  }

  CanvasPanel* GetCanvasPage(const Index& i){
    wxWindow* page = GetPage(to_size_t(i));
    assert(page != nullptr);
    CanvasPanel* canvas(dynamic_cast<CanvasPanel*>(page));
    assert(canvas != nullptr);
    return canvas;
  }

  Index GetCanvasCount() const{
    return Index(resigned(GetPageCount()));
  }

  Optional<Index> GetIndexForId(const CanvasId& id){
    const auto canvas_id_matches =
      [&](const Index& i){return GetCanvasPage(i)->GetCanvasId() == id;};

    return find_if(up_to(GetCanvasCount()), canvas_id_matches);
  }

  CanvasPanel* NewDocument(ImageProps&& props, const change_tab& changeTab,
    const initially_dirty& startDirty)
  {
    // Avoid flicker in the upper-left of the tab-bar from page
    // creation
    auto freezer = freeze(this);

    auto canvas = make_wx<CanvasPanel>(this,
      ImageList(std::move(props)),
      startDirty,
      CreateFileDropTarget(),
      m_art,
      m_app,
      m_statusInfo);
    AddPage(canvas, get_title(canvas), changeTab.Get());
    return canvas;
  }

  CanvasPanel* NewDocument(std::vector<ImageProps>&& props,
    const change_tab& changeTab, const initially_dirty& startDirty)
  {
    // Avoid flicker in the upper-left of the tab-bar from page
    // creation
    auto freezer = freeze(this);

    auto canvas = make_wx<CanvasPanel>(this,
      ImageList(std::move(props)),
      startDirty,
      CreateFileDropTarget(),
      m_art,
      m_app,
      m_statusInfo);
    AddPage(canvas, get_title(canvas), changeTab.Get());
    return canvas;
  }

  void RefreshTabName(Index i){
    CanvasPanel* canvas = GetCanvasPage(i);
    SetPageText(to_size_t(i), get_title(canvas));
  }

  void RefreshTabName(const CanvasId& id){
    RefreshTabName(GetIndexForId(id).Get());
  }

  std::unique_ptr<wxFileDropTarget> CreateFileDropTarget(){
    // Each Canvas needs a separate file drop target, as they take
    // ownership.
    return std::make_unique<CanvasFileDropTarget>(m_app);
  }
private:
  AppContext& m_app;
  const Art& m_art;
  StatusInterface& m_statusInfo;
};

TabCtrl::TabCtrl(wxWindow* parent,
  const Art& art,
  AppContext& app,
  StatusInterface& status)
{
  m_impl = make_wx<TabCtrlImpl>(parent, art, app, status);
  m_defaultTabHeight = m_impl->GetTabCtrlHeight();
}

wxWindow* TabCtrl::AsWindow(){
  return m_impl;
}

void TabCtrl::Close(const Index& i, bool force){
  assert(i < GetCanvasCount());
  m_impl->Close(i, force);
}

void TabCtrl::CloseActive(){
  m_impl->CloseActive();
}

CanvasPanel* TabCtrl::GetActiveCanvas(){
  assert(m_impl->GetPageCount() != 0);
  Index page(m_impl->GetSelection());
  CanvasPanel* canvas = m_impl->GetCanvasPage(page);
  return canvas;
}

CanvasPanel* TabCtrl::GetCanvas(const Index& page){
  assert(page < GetCanvasCount());
  CanvasPanel* cb = m_impl->GetCanvasPage(page);
  return cb;
}

CanvasPanel* TabCtrl::GetCanvas(const CanvasId& id){
  return m_impl->GetIndexForId(id).Visit(
    [&](const Index& index){
      return GetCanvas(index);
    },
    [](){
      return nullptr;
    });
}

Index TabCtrl::GetCanvasCount() const{
  return m_impl->GetCanvasCount();
}

bool TabCtrl::Has(const CanvasId& id){
  return m_impl->GetIndexForId(id).IsSet();
}

void TabCtrl::HideTabs(){
  m_defaultTabHeight = m_impl->GetTabCtrlHeight();
  m_impl->SetTabCtrlHeight(0);
}

CanvasPanel* TabCtrl::NewDocument(ImageProps&& props,
  const change_tab& changeTab,
  const initially_dirty& startDirty)
{
  return m_impl->NewDocument(std::move(props), changeTab, startDirty);
}

CanvasPanel* TabCtrl::NewDocument(std::vector<ImageProps>&& props,
  const change_tab& changeTab,
  const initially_dirty& startDirty)
{
  return m_impl->NewDocument(std::move(props), changeTab, startDirty);
}

void TabCtrl::Select(const CanvasId& id){
  auto index = m_impl->GetIndexForId(id);
  m_impl->SetSelection(to_size_t(index.Get()));
}

void TabCtrl::SelectNext(){
  m_impl->AdvanceSelection();
}

void TabCtrl::SelectPrevious(){
  m_impl->AdvanceSelection(false);
}

void TabCtrl::ShowTabs(){
  m_impl->SetTabCtrlHeight(m_defaultTabHeight);
}

bool TabCtrl::UnsavedDocuments() const{
  auto is_dirty = [this](const Index& i){
    return m_impl->GetCanvasPage(i)->IsDirty();
  };

  return any_of(up_to(GetCanvasCount()), is_dirty);
}

} // namespace

namespace faint::events{

void on_switch_canvas(window_t w, const std::function<void(CanvasId)>& f){
  bind_fwd(w.w, EVT_FAINT_ACTIVE_CANVAS_CHANGE,
    [f](CanvasChangeEvent& e){
      f(e.GetCanvasId());
    });
}

} // namespace
