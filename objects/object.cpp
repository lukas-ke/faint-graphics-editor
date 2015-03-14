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

#include <cassert>
#include "bitmap/color.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "objects/object.hh"
#include "text/utf8-string.hh"
#include "util/setting-util.hh"

namespace faint{

const Settings NullObjectSettings;

const Color mask_outside(255,255,255);
const Color mask_fill(0,255,0);
const Color mask_no_fill(0,0,255);
const Color mask_edge(255,0,0);

Object::Object(const Settings& s)
  : m_settings(s),
    m_active(false)
{
  finalize_swap_colors(m_settings);
}

bool Object::Active() const{
  return m_active;
}

bool Object::CanRemovePoint() const{
  return false;
}

void Object::ClearActive(){
  m_active = false;
}

bool Object::CyclicPoints() const{
  return false;
}

bool Object::Extendable() const{
  return false;
}

std::vector<Point> Object::GetAttachPoints() const{
  return std::vector<Point>();
}

std::vector<Point> Object::GetExtensionPoints() const{
  return std::vector<Point>();
}

ObjectId Object::GetId() const{
  return m_id;
}

std::vector<Point> Object::GetMovablePoints() const {
  return std::vector<Point>();
}

const Optional<utf8_string>& Object::GetName() const{
  return m_name;
}

Object* Object::GetObject(int) {
  assert(false);
  return nullptr;
}

const Object* Object::GetObject(int) const {
  assert(false);
  return nullptr;
}

int Object::GetObjectCount() const {
  return 0;
}

Point Object::GetPoint(int) const{
  assert(false);
  return Point(0, 0);
}

const Settings& Object::GetSettings() const{
  return m_settings;
}

std::vector<Point> Object::GetSnappingPoints() const{
  return GetAttachPoints();
}

bool Object::HitTest(const Point& p){
  return GetRefreshRect().Contains(floored(p));
}

bool Object::Inactive() const{
  return !m_active;
}

void Object::InsertPoint(const Point&, int){
  assert(false);
}

bool Object::IsControlPoint(int) const{
  return false;
}

int Object::NumPoints() const{
  return 0;
}

void Object::RemovePoint(int){
  assert(false);
}

void Object::SetActive(bool active){
  m_active = active;
}

void Object::SetName(const Optional<utf8_string>& name){
  m_name = name;
}

void Object::SetPoint(const Point&, int){
  assert(false);
}

bool Object::ShowSizeBox() const{
  return false;
}

utf8_string Object::StatusString() const{
  return utf8_string("");
}

bool Object::UpdateSettings(const Settings& s){
  return m_settings.Update(s);
}

}
