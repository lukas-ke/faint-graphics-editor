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

#ifndef FAINT_COMMAND_WINDOW_HH
#define FAINT_COMMAND_WINDOW_HH
#include "util/pos-info.hh"
#include "rendering/overlay.hh"

class wxWindow;

namespace faint{
class BitmapCommand;
class Settings;

class WindowFeedback{
  // Interface used by CommandWindows to communicate with the
  // application.
public:
  virtual ~WindowFeedback() = default;
  virtual void Closed() = 0;
  virtual void Closed(BitmapCommand*) = 0;

  virtual Bitmap GetBitmap() = 0;
  virtual void SetBitmap(const Bitmap&) = 0; // Fixme: Rename to SetPreview

  virtual void Reset() = 0; // Fixme: Do not expose this here, should not be visible to the command windows.
  virtual void UpdateSettings(const Settings&) = 0; // Fixme: Do not expose this here, should not be visible to the command windows.

  virtual void Reinitialize() = 0; // Fixme: Do not expose this here, should not be visible to the command windows.
};

class CommandWindow{
  // A non-modal (or partially modal) dialog which can create
  // Commands.
  //
  // Unlike the CommandDialog, the CommandWindow allows interaction
  // with some UI-controls while shown. This, for example, allows
  // changing the color using the palette control, possibly altering
  // the algorithm controlled by the CommandWindow.
public:
  virtual ~CommandWindow(){}
  virtual void Show(wxWindow&, WindowFeedback&) = 0;
  virtual bool IsShown() const = 0;
  virtual bool MouseMove(const PosInfo&) = 0;
  virtual bool MouseDown(const PosInfo&) = 0;
  virtual void Draw(FaintDC& dc, Overlays&, const PosInfo&) = 0;
  virtual void UpdateSettings(const Settings&) = 0;
  virtual void Reinitialize(WindowFeedback&) = 0;
  virtual const Settings& GetSettings() const = 0;

};

} // namespace

#endif
