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

#ifndef FAINT_ACTIVE_CANVAS_HH
#define FAINT_ACTIVE_CANVAS_HH
#include "app/canvas.hh"

namespace faint{

class AppContext;

class ActiveCanvas{
  // A wrapper always targetting the currently active canvas.
  //
  // This provides a narrower interface than passing around the entire
  // AppContext for AppContext::GetActiveCanvas.
public:
  explicit ActiveCanvas(AppContext&);
  Canvas& operator*();
  Canvas& operator*() const;
  Canvas* operator->();
  Canvas* operator->() const;

  ActiveCanvas& operator=(const ActiveCanvas&) = delete;
private:
  AppContext& m_app;
};

} // namespace

#endif
