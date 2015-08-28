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

#include <cassert>
#include "bitmap/color.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "objects/standard-object.hh"
#include "text/utf8-string.hh"
#include "util/setting-util.hh"

namespace faint{

StandardObject::StandardObject(const Settings& s)
  : m_settings(s),
    m_active(false)
{
  finalize_swap_colors(m_settings);
}

bool StandardObject::Active() const{
  return m_active;
}

bool StandardObject::CanRemovePoint() const{
  return false;
}

bool StandardObject::CyclicPoints() const{
  return false;
}

bool StandardObject::Extendable() const{
  return false;
}

std::vector<Point> StandardObject::GetAttachPoints() const{
  return {};
}

std::vector<ExtensionPoint> StandardObject::GetExtensionPoints() const{
  return {};
}

ObjectId StandardObject::GetId() const{
  return m_id;
}

std::vector<Point> StandardObject::GetMovablePoints() const {
  return {};
}

const Optional<utf8_string>& StandardObject::GetName() const{
  return m_name;
}

Object* StandardObject::GetObject(int) {
  assert(false);
  return nullptr;
}

const Object* StandardObject::GetObject(int) const {
  assert(false);
  return nullptr;
}

int StandardObject::GetObjectCount() const {
  return 0;
}

Point StandardObject::GetPoint(int) const{
  assert(false);
  return Point(0, 0);
}

const Settings& StandardObject::GetSettings() const{
  return m_settings;
}

Settings& StandardObject::GetSettings(){
  return m_settings;
}

std::vector<Point> StandardObject::GetSnappingPoints() const{
  return GetAttachPoints();
}

bool StandardObject::HitTest(const Point& p){
  return GetRefreshRect().Contains(floored(p));
}

UndoAddFunc StandardObject::InsertPoint(const Point&, int){
  assert(false);
  return [](){};
}

bool StandardObject::IsControlPoint(int) const{
  return false;
}

int StandardObject::NumPoints() const{
  return 0;
}

Optional<CmdFuncs> StandardObject::PixelSnapFunc(){
  return {};
}

void StandardObject::RemovePoint(int){
  assert(false);
}

void StandardObject::SetActive(bool active){
  m_active = active;
}

void StandardObject::SetName(const Optional<utf8_string>& name){
  m_name = name;
}

void StandardObject::SetPoint(const Point&, int){
  assert(false);
}

bool StandardObject::ShowSizeBox() const{
  return false;
}

utf8_string StandardObject::StatusString() const{
  return utf8_string("");
}

bool StandardObject::UpdateSettings(const Settings& s){
  return m_settings.Update(s);
}

} // namespace
