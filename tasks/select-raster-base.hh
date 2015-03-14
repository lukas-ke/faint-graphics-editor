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

#ifndef FAINT_SELECT_RASTER_BASE_HH
#define FAINT_SELECT_RASTER_BASE_HH
#include "tasks/standard-task.hh"

namespace faint{

class Canvas;

class RasterSelectionTask : public StandardTask {
  // Base for common updating of the selection settings from any
  // raster selection task.
public:
  RasterSelectionTask() = default;
  void UpdateSettings() final override;
  bool EatsSettings() const final override;
  RasterSelectionTask& operator=(const RasterSelectionTask&) = delete;
private:
  virtual Canvas& GetCanvas() const = 0;

  virtual const Settings& GetSettings() = 0;
};

} // namespace

#endif
