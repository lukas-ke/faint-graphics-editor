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

#include "bitmap/bitmap.hh"
#include "objects/object.hh"
#include "util/frame-props.hh"
#include "util/index.hh"
#include "util/object-util.hh"

namespace faint{

ImageInfo::ImageInfo(const IntSize& size, const create_bitmap& createBitmap)
  : size(size),
    backgroundColor(color_white),
    createBitmap(createBitmap.Get())
{}

ImageInfo::ImageInfo(const IntSize& size,
  const Color& bgCol,
  const create_bitmap& createBitmap)
  : size(size),
    backgroundColor(bgCol),
    createBitmap(createBitmap.Get())
{}

FrameInfo::FrameInfo()
  : delay(0),
    hotSpot(0,0)
{}

FrameInfo::FrameInfo(const Delay& delay)
  : delay(delay),
    hotSpot(0,0)
{}

FrameInfo::FrameInfo(const HotSpot& hotSpot)
  : delay(0),
    hotSpot(hotSpot)
{}

FrameInfo::FrameInfo(const Delay& delay, const HotSpot& hotSpot)
  : delay(delay),
    hotSpot(hotSpot)
{}

FrameProps::FrameProps()
  : m_background(ColorSpan(color_white, IntSize(640,480))),
    m_delay(0)
{}

FrameProps::FrameProps(const Bitmap& bmp)
  : m_background(bmp),
    m_delay(0),
    m_hotSpot(0,0)
{
  assert(bitmap_ok(bmp));
}

FrameProps::FrameProps(const Bitmap& bmp, const objects_t& objects)
  : m_background(bmp),
    m_delay(0),
    m_hotSpot(0,0),
    m_objects(objects)
{
  m_delay = 0;
}

FrameProps::FrameProps(const Bitmap& bmp, const FrameInfo& info)
  : m_background(bmp),
    m_delay(info.delay),
    m_hotSpot(info.hotSpot)
{}

FrameProps::FrameProps(const IntSize& size, const objects_t& objects)
  : m_background(ColorSpan(color_white, size)),
    m_delay(0),
    m_objects(objects)
{}

FrameProps::FrameProps(const ImageInfo& info)
  : m_background(ColorSpan(info.backgroundColor, info.size)),
    m_delay(0)
{
  if (info.createBitmap){
    assert(info.size.w > 0 && info.size.h > 0);
    m_background.Set(Bitmap(info.size, info.backgroundColor));
  }
}

FrameProps::FrameProps(FrameProps&& other)
  : m_allObjects(std::move(other.m_allObjects)),
    m_background(std::move(other.m_background)),
    m_delay(other.m_delay),
    m_hotSpot(other.m_hotSpot),
    m_objects(std::move(other.m_objects))
{}

FrameProps::~FrameProps(){
  for (Object* obj : m_objects){
    // Not deleting from m_allObjects, since the ObjComposites
    // are responsible for deleting sub-objects.
    delete obj;
  }

  m_objects.clear();
  m_allObjects.clear();
}

Index FrameProps::AddObject(Object* obj){
  m_objects.push_back(obj);
  m_allObjects.push_back(obj);
  return to_index(m_allObjects.size() - 1);
}

const Either<Bitmap, ColorSpan>& FrameProps::GetBackground() const{
  return m_background;
}

const Optional<Calibration>& FrameProps::GetCalibration() const{
  return m_calibration;
}

Delay FrameProps::GetDelay() const{
  return m_delay;
}

HotSpot FrameProps::GetHotSpot() const{
  return m_hotSpot;
}

Object* FrameProps::GetObject(const Index& index){
  return m_allObjects.at(to_size_t(index));
}

bool FrameProps::HasObject(const Index& index) const{
  return index < to_index(m_allObjects.size());
}

bool FrameProps::IsTopLevel(const Index& index) const{
  assert(HasObject(index));
  Object* obj = m_allObjects.at(to_size_t(index));
  return contains(m_objects, obj);
}

void FrameProps::RemoveObject(Object* obj){
  bool removed = remove(obj, from(m_objects));
  assert(removed);
}

void FrameProps::SetBackground(const Either<Bitmap, ColorSpan>& bg){
  m_background = bg;
  m_delay = 0;
  m_hotSpot = HotSpot(0,0);
}

void FrameProps::SetCalibration(const Calibration& c){
  m_calibration.Set(c);
}

objects_t FrameProps::TakeObjects(){
  objects_t objects(std::move(m_objects));
  assert(m_objects.empty());
  return objects;
}

} // namespace
