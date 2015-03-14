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

#ifndef FAINT_COLOR_PANEL_HH
#define FAINT_COLOR_PANEL_HH
#include "util/dumb-ptr.hh"

class wxWindow;

namespace faint{

class AppContext;
class ArtContainer;
class ColorChoice;
class ColorPanelImpl;
class Paint;
class PaintMap;
class StatusInterface;
class Settings;
class ZoomLevel;

class ColorPanel {
  // Horizontal panel containing a selected color control, a palette
  // control, a zoom control and an animation frames control.
public:
  ColorPanel(wxWindow* parent,
    const Settings&,
    const PaintMap& palette,
    AppContext&,
    StatusInterface&,
    const ArtContainer&);
  void AddToPalette(const Paint&);
  wxWindow* AsWindow();
  void Freeze();
  void Hide();
  void Show(bool);
  void Thaw();
  void UpdateFrames();
  void UpdateGrid();
  void UpdateSelectedColors(const ColorChoice&);
  void UpdateZoom(const ZoomLevel&);
  void SetPalette(const PaintMap&);
  bool Visible() const;

  ColorPanel(const ColorPanel&) = delete;
private:
  // Using impl instead of inheriting large interface
  dumb_ptr<ColorPanelImpl> m_impl;
};

} // namespace

#endif
