// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_MOUSE_CAPTURE_HH
#define FAINT_MOUSE_CAPTURE_HH
#include <functional>

class wxWindow;

namespace faint{

// Fixme: Add dummy parameters to capture lost, on-release,
// to avoid incorrect ordering.

class MouseCapture{
  // Manages wxWidgets mouse capturing for a window. Releases capture
  // on destruction, if necessary.
  //
  // The function passed to the constructor is called when capture is
  // lost (e.g. due to focus changes). If no function is passed,
  // lost-capture will have no effects.
public:
  MouseCapture(wxWindow*);
  MouseCapture(wxWindow*, const std::function<void()>& onCaptureLost);
  MouseCapture(wxWindow*,
    const std::function<void()>& onCaptureLost,
    const std::function<void()>& onRelease);
  ~MouseCapture();
  void Capture();
  bool HasCapture() const;
  void Release();
private:
  std::function<void()> m_onCaptureLost;
  std::function<void()> m_onRelease;
  wxWindow* m_window;
};

inline auto releaser(MouseCapture& capture){
  return [&](auto...){capture.Release();};
}

} // namespace

#endif
