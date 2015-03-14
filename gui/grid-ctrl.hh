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

#ifndef FAINT_GRID_CTRL_HH
#define FAINT_GRID_CTRL_HH
#include <memory>
#include <vector>
#include "wx/panel.h"
#include "util/optional.hh"

class wxButton;

namespace faint{

class ArtContainer;
class DragValueCtrl;
class SpinButton;
class StatusInterface;

using winvec_t = std::vector<wxWindow*>;

class GridCtrl : public wxPanel {
public:
  GridCtrl(wxWindow* parent, const ArtContainer&, StatusInterface&);

  // Update the control to match external changes e.g. canvas change
  // or grid disabled via Python
  void Update();

private:
  void EnableGrid(bool);
  const ArtContainer& m_art;
  wxButton* m_btnToggle;
  bool m_enabled;
  Optional<int> m_newValue;
  std::vector<wxWindow*> m_showhide;
  wxSizer* m_sizer;
  std::unique_ptr<SpinButton> m_spinButton;
  DragValueCtrl* m_txtCurrentSize;
};

} // namespace

#endif
