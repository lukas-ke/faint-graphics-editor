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

#ifndef FAINT_TAB_CTRL_HH
#define FAINT_TAB_CTRL_HH
#include "app/app-context.hh"
#include "gui/canvas-panel.hh" // Fixme: For initially_dirty
#include "wx/event.h" // Fixme: Hide in impl? (events::on...)

namespace faint{

class ArtContainer;
class CanvasChangeEvent;
class CanvasPanel;

extern const wxEventTypeTag<CanvasChangeEvent> EVT_FAINT_ACTIVE_CANVAS_CHANGE;

class TabCtrlImpl;

class TabCtrl {
public:
  TabCtrl(wxWindow* parent,
    const ArtContainer&,
    AppContext&,
    StatusInterface&);
  wxWindow* AsWindow();
  void Close(const Index&);
  void CloseActive();
  CanvasPanel* GetActiveCanvas();
  CanvasPanel* GetCanvas(const Index&);
  Index GetCanvasCount() const;
  bool Has(const CanvasId&);
  void HideTabs();
  CanvasPanel* NewDocument(ImageProps&&, const change_tab&,
    const initially_dirty&);
  CanvasPanel* NewDocument(std::vector<ImageProps>&&, const change_tab&,
    const initially_dirty&);
  void Select(const CanvasId&);
  void SelectNext();
  void SelectPrevious();
  void ShowTabs();
  bool UnsavedDocuments() const;
private:
  TabCtrl(const TabCtrl&);
  TabCtrlImpl* m_impl;
  int m_defaultTabHeight;
};

} // namespace

#endif
