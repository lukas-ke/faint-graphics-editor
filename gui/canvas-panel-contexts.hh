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

#ifndef FAINT_CANVAS_PANEL_CONTEXTS_HH
#define FAINT_CANVAS_PANEL_CONTEXTS_HH
#include <memory>
#include "commands/command.hh"

namespace faint{

class AppContext;
class Canvas;
class CanvasPanel;
class ImageList;

class TargetableCommandContext : public CommandContext{
public:
  virtual void SetFrame(Image*) = 0;
  virtual void RevertFrame() = 0;
};

std::unique_ptr<TargetableCommandContext> create_command_context(CanvasPanel&,
  ImageList&);

std::unique_ptr<Canvas> create_canvas_context(CanvasPanel&,
  AppContext&,
  const ImageList&);

} // namespace

#endif
