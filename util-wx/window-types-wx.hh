// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_WINDOW_TYPES_WX_HH
#define FAINT_WINDOW_TYPES_WX_HH
#include <memory>

class wxBitmap;
class wxBookCtrlBase;
class wxButton;
class wxCheckBox;
class wxChoice;
class wxDialog;
class wxPanel;
class wxSizer;
class wxStaticText;
class wxTextCtrl;
class wxWindow;

namespace faint{

using delete_dialog_f = void(*)(wxDialog*);
using unique_dialog_ptr = std::unique_ptr<wxDialog, delete_dialog_f>;

class window_t{
  // Type which many wxWidgets window types convert to implicitly, to
  // allow passing forward-declared wxWidgets-classes to functions,
  // avoiding including the full definition.
  //
  // Note: Defined in fwd-wx.cpp
public:
  window_t(wxDialog*);
  window_t(unique_dialog_ptr&);
  window_t(wxWindow&);
  window_t(wxBookCtrlBase*);
  window_t(wxWindow*);
  window_t(wxStaticText*);
  window_t(wxPanel*);
  window_t(wxCheckBox*);
  window_t(wxTextCtrl*);
  window_t(wxButton*);
  window_t(wxChoice*);
  wxWindow* w;
};

} // namespace

#endif
