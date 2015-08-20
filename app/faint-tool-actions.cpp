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

#include "app/app-context.hh"
#include "app/faint-tool-actions.hh"

namespace faint{

FaintToolActions::FaintToolActions(AppContext& app)
  : m_app(app)
{}

void FaintToolActions::BeginTextEntry(){
  m_app.BeginTextEntry();
}

void FaintToolActions::EndTextEntry(){
  m_app.EndTextEntry();
}

void FaintToolActions::Set(const BoolSetting& s, BoolSetting::ValueType v){
  m_app.Set(s, v);
}

void FaintToolActions::Set(const StringSetting& s,
  const StringSetting::ValueType& v)
{
  m_app.Set(s, v);
}

void FaintToolActions::Set(const IntSetting& s, IntSetting::ValueType v){
  m_app.Set(s, v);
}

void FaintToolActions::Set(const PaintSetting& s, PaintSetting::ValueType v){
  m_app.Set(s, v);
}

void FaintToolActions::Set(const FloatSetting& s, FloatSetting::ValueType v){
  m_app.Set(s, v);
}

void FaintToolActions::UpdateShownSettings(){
  m_app.UpdateShownSettings();
}

} // namespace
