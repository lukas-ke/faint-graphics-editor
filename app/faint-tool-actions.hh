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

#ifndef FAINT_FAINT_TOOL_ACTIONS_HH
#define FAINT_FAINT_TOOL_ACTIONS_HH
#include "tools/tool-actions.hh"

namespace faint{

class AppContext;

class FaintToolActions : public ToolActions {
public:
  FaintToolActions(AppContext&);
  void BeginTextEntry() override;
  void EndTextEntry() override;
  void Set(const BoolSetting&, BoolSetting::ValueType) override;
  void Set(const StringSetting&, const StringSetting::ValueType&) override;
  void Set(const IntSetting&, IntSetting::ValueType) override;
  void Set(const PaintSetting&, PaintSetting::ValueType) override;
  void Set(const FloatSetting&, FloatSetting::ValueType) override;

private:
  AppContext& m_app;
};

} // namespace

#endif
