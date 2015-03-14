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

#include <algorithm>
#include <cassert>
#include "geo/primitive.hh"
#include "objects/object.hh"
#include "text/text-expression-context.hh"
#include "util/frame-props.hh"
#include "util/image.hh"
#include "util/object-util.hh"

namespace faint{

// Find where in target to insert to keep the relative sort order of source
static size_t get_sorted_insertion_pos(Object* obj,
  const objects_t& v_trg,
  const objects_t& v_src)
{
  size_t i_src = 0;
  size_t i_trg = 0;
  for (; i_trg != v_trg.size(); i_trg++){
    const Object* o_trg = v_trg[i_trg];

    for (; i_src!= v_src.size(); i_src++){
      const Object* o_src = v_src[i_src];
      if (o_src == obj){
        // Object to be inserted found before reference object ->
        // this is the insertion point.
        return i_trg;
      }
      if (o_src == o_trg){
        // Reference object found in destination vector -
        // inserted object must be later in the source vector.
        break;
      }
    }
  }
  return i_trg;
}

class ImageExpressionContext : public ExpressionContext{
public:
  ImageExpressionContext(Image* image)
    : m_image(image)
 {}

  Optional<Calibration> GetCalibration() const override{
    return m_image->GetCalibration();
  }

  const Object* GetObject(const utf8_string& name) const override{
    return get_by_name(m_image->GetObjects(), name);
  }

private:
  Image* m_image;
};

Image::Image(FrameProps&& props)
  : m_bg(std::move(props.GetBackground())),
    m_calibration(props.GetCalibration()),
    m_delay(props.GetDelay()),
    m_hotSpot(props.GetHotSpot()),
    m_objects(props.TakeObjects()),
    m_original(),
    m_originalObjects(m_objects)
{
  m_expressionContext = new ImageExpressionContext(this);
}

Image::Image(const Image& other)
  : m_bg(other.m_bg),
    m_delay(other.GetDelay()),
    m_hotSpot(other.m_hotSpot),
    m_original()
{
  m_originalObjects = m_objects = clone(other.GetObjects());
  m_expressionContext = new ImageExpressionContext(this);
}

Image::Image()
  : m_bg(ColorSpan(color_white, IntSize(1,1))),
    m_delay(0)
{
  m_expressionContext = new ImageExpressionContext(this);
}

Bitmap& Image::ConvertColorSpanToBitmap(){
  const ColorSpan& span(m_bg.Expect<ColorSpan>());
  m_original.Set(span);
  m_bg.Set(Bitmap(span.size, span.color));
  return m_bg.Expect<Bitmap>();
}

bool Image::HasStoredOriginal() const{
  return m_original.IsSet();
}

void Image::StoreAsOriginal(){
  assert(m_original.NotSet());
  m_bg.Visit(
    [&](const Bitmap& bmp){
      m_original.Set(bmp);
    },
    [&](const ColorSpan& span){
      m_original.Set(span);
    });
}

void Image::SetBitmap(const Bitmap& bmp){
  m_bg.Set(bmp);
}

void Image::SetBitmap(Bitmap&& bmp){
  m_bg.Set(std::move(bmp));
}

Image::~Image(){
  // Delete the objects the image was created with - any other objects
  // are deleted when the corresponding AddObject-commands are
  // deleted.
  for (Object* obj : m_originalObjects){
    delete obj;
  }
  delete m_expressionContext;
}

ExpressionContext& Image::GetExpressionContext() const{
  return *m_expressionContext;
}

FrameId Image::GetId() const{
  return m_id;
}

IntSize Image::GetSize() const{
  return m_bg.Visit(
    [](const Bitmap& bmp){
      return bmp.GetSize();
    },
    [](const ColorSpan& span){
      return span.size;
    });
}

void Image::Add(Object* object){
  assert(!Has(object));
  m_objects.push_back(object);
}

void Image::Add(Object* object, int z){
  assert(!Has(object));
  assert(z >= 0);
  assert(to_size_t(z) <= m_objects.size());
  m_objects.insert(begin(m_objects) + z, object);
}

bool Image::Deselect(const Object* object){
  return remove(object, from(m_objectSelection));
}

bool Image::Deselect(const objects_t& objects){
  return remove(objects, from(m_objectSelection));
}

void Image::DeselectObjects(){
  m_objectSelection.clear();
}

const Optional<Calibration>& Image::GetCalibration() const{
  return m_calibration;
}

const objects_t& Image::GetObjects() const{
  return m_objects;
}

const objects_t& Image::GetObjectSelection() const{
  return m_objectSelection;
}

int Image::GetObjectZ(const Object* obj) const{
  for (size_t i = 0; i != m_objects.size(); i++){
    if (m_objects[i] == obj){
      return resigned(i);
    }
  }
  assert(false);
  return 0;
}

RasterSelection& Image::GetRasterSelection(){
  return m_rasterSelection;
}

const RasterSelection& Image::GetRasterSelection() const{
  return m_rasterSelection;
}

void Image::SelectObjects(const objects_t& objects){
  for (Object* obj : objects){
    if (lacks(m_objectSelection, obj)){
      size_t pos = get_sorted_insertion_pos(obj, m_objectSelection, m_objects);
      m_objectSelection.insert(begin(m_objectSelection) + resigned(pos), obj);
    }
  }
}

void Image::SetCalibration(const Optional<Calibration>& c){
  m_calibration = c;
}

void Image::SetObjectZ(Object* obj, int z){
  bool wasSelected = contains(m_objectSelection, obj);
  Remove(obj);
  z = std::min(z, resigned(m_objects.size()));
  m_objects.insert(begin(m_objects) + z, obj);
  if (wasSelected){
    size_t pos = get_sorted_insertion_pos(obj, m_objectSelection, m_objects);
    m_objectSelection.insert(begin(m_objectSelection) + resigned(pos), obj);
  }
}

void Image::Remove(Object* obj){
  remove(obj, from(m_objectSelection));
  bool removed = remove(obj, from(m_objects));
  assert(removed);
}

int Image::GetNumObjects() const{
  return resigned(m_objects.size());
}

bool Image::Has(const ObjectId& objId) const{
  for (const Object* obj : m_objects){
    if (is_or_has(obj, objId)){
      return true;
    }
  }
  return false;
}

bool Image::Has(const Object* obj) const{
  return contains(m_objects, obj);
}

void Image::Revert(){
  m_original.Visit(
    [&](const Either<Bitmap, ColorSpan>& bg){
      m_bg = bg;
    },
    [](){
      assert(false);
    });
}

const Either<Bitmap, ColorSpan>& Image::GetBackground() const{
  return m_bg;
}

FrozenEither<Bitmap, ColorSpan> Image::GetBackground(){
  return frozen(m_bg);
}

Delay Image::GetDelay() const{
  return m_delay;
}

HotSpot Image::GetHotSpot() const{
  return m_hotSpot;
}

void Image::SetDelay(const Delay& delay){
  m_delay = delay;
}

void Image::SetHotSpot(const HotSpot& hotSpot){
  m_hotSpot = hotSpot;
}

IntRect image_rect(const Image& image){
  return IntRect(IntPoint(0,0), image.GetSize());
}

} // namespace faint
