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

#include "wx/button.h"
#include "wx/dialog.h"
#include "wx/sizer.h"
#include "wx/stattext.h"
#include "gui/layout.hh"
#include "util-wx/layout-wx.hh"
#include "util/iter.hh"

namespace faint{namespace layout{

const int EXPAND = wxEXPAND;
const int DOWN = wxDOWN;
const int UP = wxUP;
const int ALIGN_RIGHT = wxALIGN_RIGHT;

SizerItem::SizerItem(wxWindow* item, Proportion p, int flags, int border)
  : border(border),
    flags(flags),
    proportion(p),
    windowItem(item)
{}

SizerItem::SizerItem(wxWindow* item)
  : flags(wxALIGN_CENTER_VERTICAL),
    windowItem(item)
{}

SizerItem::SizerItem(wxButton* item)
  : SizerItem(static_cast<wxWindow*>(item))
{}

SizerItem::SizerItem(wxStaticText* item)
  : SizerItem(static_cast<wxWindow*>(item))
{}

SizerItem::SizerItem(wxSizer* item, Proportion p, int flags, int border)
  : border(border),
    flags(flags),
    proportion(p),
    sizerItem(item)
{}

SizerItem::SizerItem(wxSizer* item)
  : flags(wxALIGN_CENTER_VERTICAL),
    sizerItem(item)
{}

SizerItem::SizerItem(wxButton* item, int flags)
  : flags(flags),
    windowItem(item)
{}

bool add(wxBoxSizer* sizer, const SizerItem& item){
  if (item.windowItem != nullptr){
    sizer->Add(item.windowItem, item.proportion.Get(), item.flags, item.border);
    return true;
  }
  else if (item.sizerItem != nullptr){
    sizer->Add(item.sizerItem, item.proportion.Get(), item.flags, item.border);
    return true;
  }
  return false;
}

wxSizer* create_row(const std::vector<SizerItem>& items){
  wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);

  row->AddSpacer(panel_padding);
  for (auto& item : but_last(items)){
    if (add(row, item)){
      row->AddSpacer(item_spacing);
    }
  }
  add(row, items.back());
  row->AddSpacer(panel_padding);
  return row;
}

wxSizer* create_row_outer_pad(const std::vector<SizerItem>& items){
  wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
  row->AddSpacer(panel_padding);
  for (auto& item : but_last(items)){
    add(row, item);
  }
  add(row, items.back());
  row->AddSpacer(panel_padding);
  return row;
}

wxSizer* create_row_no_pad(const std::vector<SizerItem>& items){
  wxBoxSizer* row = new wxBoxSizer(wxHORIZONTAL);
  for (auto& item : items){
    if (item.windowItem != nullptr){
      row->Add(item.windowItem, item.proportion.Get(), item.flags, item.border);
    }
    else{
      row->Add(item.sizerItem, item.proportion.Get(), item.flags, item.border);
    }
  }
  return row;
}

wxSizer* create_column(OuterSpacing os, ItemSpacing is,
  const std::vector<SizerItem>& items)
{
  wxBoxSizer* column = new wxBoxSizer(wxVERTICAL);
  const auto outerSpacing = os.Get();
  const auto itemSpacing = is.Get();
  if (outerSpacing != 0){
    column->AddSpacer(outerSpacing);
  }

  for (auto& item : but_last(items)){
    if (item.windowItem != nullptr){
      column->Add(item.windowItem, item.proportion.Get(), item.flags, item.border);
    }
    else{
      column->Add(item.sizerItem, item.proportion.Get(), item.flags, item.border);
    }
    if (itemSpacing != 0){
      column->AddSpacer(itemSpacing);
    }
  }
  auto& last = items.back();
  if (last.windowItem != nullptr){
    column->Add(last.windowItem, last.proportion.Get(), last.flags, last.border);
  }
  else{
    column->Add(last.sizerItem, last.proportion.Get(), last.flags, last.border);
  }
  if (outerSpacing != 0){
    column->AddSpacer(outerSpacing);
  }

  return column;
}

wxSizer* create_column(const std::vector<SizerItem>& items){
  return create_column(OuterSpacing(panel_padding),
    ItemSpacing(item_spacing),
    items);

}
SizerItem grow(wxSizer* sizer){
  return SizerItem(sizer, 1, wxEXPAND);
}

SizerItem grow(wxWindow* window){
  return SizerItem(window, 1, wxEXPAND);
}

SizerItem center(wxSizer* sizer){
  return SizerItem(sizer, 0, wxALIGN_CENTER);
}

wxWindow* make_default(wxDialog* parent, wxWindow* item){
  parent->SetDefaultItem(item);
  return item;
}

wxButton* make_default(wxDialog* parent, wxButton* item){
  parent->SetDefaultItem(item);
  return item;
}

wxStaticText* label(wxWindow* parent, const wxString& text){
  return new wxStaticText(parent, wxID_ANY, text);
}

}} // namespace
