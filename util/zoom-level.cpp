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
#include "util/zoom-level.hh"

namespace faint{

static const int defaultLevel = 0;
static const int positiveZoom[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 20, 30, 40 };
static const int maxZoom = sizeof(positiveZoom) / sizeof(int) - 1;
static const coord negativeZoom[] = {
  0.9,
  0.8,
  0.7,
  0.6,
  0.5,
  0.4,
  0.3,
  0.2,
  0.1
};
static const int negativeZoomPct[] = {90, 80, 70, 60, 50, 40, 30, 20, 10 };
static const int minZoom = - (int((sizeof(negativeZoom) / sizeof(coord))));

int find_closest(coord scale){
  if (scale < 1.0){
    for (int i = 0; i != 9; i++){
      if (scale > negativeZoom[i]){
        return(-i - 1);
      }
    }
    return minZoom;
  }

  for (int i = 0; i != maxZoom; i++){
    if (scale < positiveZoom[i]){
      return i - 1;
    }
  }
  return maxZoom;
}

ZoomLevel::ZoomLevel(){
  SetLevel(defaultLevel);
}

bool ZoomLevel::At100() const{
  return !m_scale.IsSet() && m_zoomLevel == defaultLevel;
}

bool ZoomLevel::AtMax() const{
  if (m_scale.IsSet()){
    return find_closest(m_scale.Get()) == maxZoom;
  }
  return m_zoomLevel == maxZoom;
}

bool ZoomLevel::AtMin() const{
  if (m_scale.IsSet()){
    return find_closest(m_scale.Get()) == minZoom;
  }
  return m_zoomLevel == minZoom;
}

bool ZoomLevel::Change(ZoomLevel::ChangeType type){
  if (type == PREVIOUS){
    return Prev();
  }
  else if (type == NEXT){
    return Next();
  }
  else if (type == DEFAULT){
    if (m_zoomLevel == defaultLevel && !m_scale.IsSet()){
      return false;
    }
    SetLevel(defaultLevel);
    return true;
  }
  assert(false);
  return false;
}

int ZoomLevel::GetPercentage() const{
  if (m_scale.IsSet()){
    return static_cast<int>(m_scale.Get() * 100);
  }
  if (m_zoomLevel >= 0){
    return positiveZoom[ m_zoomLevel ] * 100;
  }
  return negativeZoomPct[-m_zoomLevel - 1];
}

coord ZoomLevel::GetScaleFactor() const{
  if (m_scale.IsSet()){
    return m_scale.Get();
  }
  if (m_zoomLevel >= 0){
    return static_cast<coord>(positiveZoom[ m_zoomLevel ]);
  }
  return negativeZoom[ -m_zoomLevel - 1 ];
}

bool ZoomLevel::Next(){
  if (m_scale.IsSet()){
    int level = find_closest(m_scale.Get());
    m_zoomLevel = level;
    m_scale.Clear();
  }

  if (m_zoomLevel == maxZoom){
    return false;
  }
  m_zoomLevel += 1;
  return true;
}

bool ZoomLevel::Prev(){
  if (m_scale.IsSet()){
    int level = find_closest(m_scale.Get());
    m_zoomLevel = level;
    m_scale.Clear();
  }

  if (m_zoomLevel == minZoom){
    return false;
  }
  m_zoomLevel -= 1;
  return false;
}

void ZoomLevel::SetApproximate(coord scale){
  if (scale < 1.0){
    m_scale.Set(scale);
    return;
  }
  for (int i = 0; i != maxZoom; i++){
    if (scale < positiveZoom[i]){
      SetLevel(i - 1);
      return;
    }
  }
  SetLevel(maxZoom);
}

void ZoomLevel::SetLevel(int level){
  assert(level >= minZoom);
  assert(level <= maxZoom);
  m_scale.Clear();
  m_zoomLevel = level;
}

} // namespace
