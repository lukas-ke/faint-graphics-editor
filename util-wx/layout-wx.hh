// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_LAYOUT_WX_HH
#define FAINT_LAYOUT_WX_HH
#include <vector>
#include "util/distinct.hh"

class wxBoxSizer;
class wxButton;
class wxDialog;
class wxSizer;
class wxStaticText;
class wxString;
class wxWindow;

namespace faint{ namespace layout{

class category_layout_wx;
using Proportion = Distinct<int, category_layout_wx, 0>;
using OuterSpacing = Distinct<int, category_layout_wx, 1>;
using ItemSpacing = Distinct<int, category_layout_wx, 2>;

extern const int EXPAND;
extern const int DOWN;
extern const int UP;
extern const int ALIGN_RIGHT;

class SizerItem{
public:
  SizerItem(wxWindow*, Proportion, int flags, int border=0);
  SizerItem(wxSizer*, Proportion, int flags, int border=0);
  SizerItem(wxWindow*);
  SizerItem(wxButton*);
  SizerItem(wxButton*, int flags);
  SizerItem(wxStaticText*);
  SizerItem(wxSizer*);

  int border = 0;
  int flags = 0;
  Proportion proportion = Proportion(0);
  wxSizer* sizerItem = nullptr;
  wxWindow* windowItem = nullptr;
};

// Adds the item to the sizer.
// Note: Prefer using create_row etc.
bool add(wxBoxSizer*, const SizerItem&);

wxSizer* create_row(const std::vector<SizerItem>&);
wxSizer* create_row_outer_pad(const std::vector<SizerItem>&);
wxSizer* create_row_no_pad(const std::vector<SizerItem>&);

wxSizer* create_column(OuterSpacing, ItemSpacing, const std::vector<SizerItem>&);
wxSizer* create_column(const std::vector<SizerItem>&);
SizerItem grow(wxSizer*);
SizerItem grow(wxWindow*);
SizerItem center(wxSizer*);

wxButton* make_default(wxDialog*, wxButton*);

wxStaticText* label(wxWindow* parent, const wxString&);

// Sets the focus to the passed in window, and returns it, to allow
// chaining in create_row etc.
template<typename T>
T* focused(T* obj){
  obj->SetFocus();
  return obj;
}

}} // Namespace

#endif
