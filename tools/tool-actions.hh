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

#ifndef FAINT_TOOL_ACTIONS_HH
#define FAINT_TOOL_ACTIONS_HH

#include "util/settings.hh"

namespace faint{

class ToolActions{
  // Interface for actions a tool is permitted to do without a
  // CommandContext. These must not modify the Image.
public:
  virtual void BeginTextEntry() = 0;
  virtual void EndTextEntry() = 0;
  virtual void Set(const BoolSetting&, BoolSetting::ValueType) = 0;
  virtual void Set(const StringSetting&, const StringSetting::ValueType&) = 0;
  virtual void Set(const IntSetting&, IntSetting::ValueType) = 0;
  virtual void Set(const PaintSetting&, PaintSetting::ValueType) = 0;
  virtual void Set(const FloatSetting&, FloatSetting::ValueType) = 0;
  virtual void UpdateShownSettings() = 0;
};

} // namespace

#endif
