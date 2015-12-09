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
#include "util-wx/window-types-wx.hh"
#include "app/app-context.hh"
#include "gui/canvas-panel.hh"

namespace faint{

class Art;
class CanvasChangeEvent;
class CanvasPanel;

class TabCtrlImpl;

class TabCtrl {
public:
  TabCtrl(wxWindow* parent,
    const Art&,
    AppContext&,
    StatusInterface&);
  wxWindow* AsWindow();
  void Close(const Index&, bool force);
  void CloseActive();
  CanvasPanel* GetActiveCanvas();
  CanvasPanel* GetCanvas(const Index&);
  CanvasPanel* GetCanvas(const CanvasId&);
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

namespace faint{ namespace events{

void on_switch_canvas(window_t, const std::function<void(CanvasId)>&);

}} // namespace

#endif
