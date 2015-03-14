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

#ifndef FAINT_RESIZE_WINDOW_HH
#define FAINT_RESIZE_WINDOW_HH
#include "geo/int-size.hh"
#include "geo/size.hh"
#include "gui/dialog-context.hh"
#include "gui/dialogs/resize-dialog-options.hh"
#include "text/utf8-string.hh"
#include "util/either.hh"

namespace faint{

class ArtContainer;
class IntRect;
class Paint;
enum class ScaleQuality;

class ResizeDialogContext{
  // Interface used by the Resize dialog to get info about the
  // state/image, and to perform its action.
public:
  virtual ~ResizeDialogContext() = default;

  virtual bool AllowNearestNeighbour() const = 0;
  virtual bool AllowResize() const = 0;

  // Background for new regions when growing the image (resize)
  virtual Paint GetBackground() const = 0;
  virtual ResizeDialogOptions GetOptions() const = 0;
  virtual Either<IntSize, Size> GetSize() const = 0;
  virtual utf8_string GetTitle() const = 0;

  virtual void Resize(const IntRect&, const Paint& bg) = 0;
  virtual void Rescale(Size, ScaleQuality) = 0;

  // Set the options that should be initially selected when the
  // ResizeDialog is shown again.
  virtual void SetDefaultOptions(const ResizeDialogOptions&) = 0;

  // Returns true if the size is acceptable
  virtual bool SupportedSize(const Size&) = 0;
};

void show_resize_dialog(DialogContext&,
  const ArtContainer&,
  std::unique_ptr<ResizeDialogContext>);

} // namespace

#endif
