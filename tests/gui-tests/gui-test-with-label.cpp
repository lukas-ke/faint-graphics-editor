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

#include "bitmap/bitmap.hh"
#include "gui/with-label.hh"
#include "wx/button.h"
#include "wx/textctrl.h"

namespace faint{ class StatusInterface; }
namespace faint{ class DialogContext; }

void gui_test_with_label(wxWindow* p, faint::StatusInterface&,
  faint::DialogContext&)
{
  using namespace faint;
  wxBoxSizer* sz = new wxBoxSizer(wxVERTICAL);
  auto add = [&sz](wxWindow* w){sz->Add(w, 0, wxDOWN, 20);};

  add(label_left<wxButton>(p, "Label", wxID_ANY, "Button"));

  add(label_left<wxTextCtrl>(p, "Label", wxID_ANY, "TextCtrl"));

  add(label_below<wxTextCtrl>(p, "Label", wxID_ANY, "TextCtrl"));

  add(label_below<wxButton>(p, "Label", wxID_ANY, "Button"));

  p->SetSizerAndFit(sz);
}
