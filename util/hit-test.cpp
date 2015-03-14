// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "geo/geo-func.hh"
#include "geo/geo-list-points.hh"
#include "geo/handle-func.hh"
#include "geo/line.hh"
#include "geo/size.hh"
#include "objects/object.hh"
#include "rendering/faint-dc.hh"
#include "util/hit-test.hh"
#include "util/image.hh"
#include "util/iter.hh"
#include "util/mouse.hh"
#include "util/object-util.hh"

namespace faint{

static std::pair<Object*, Hit> object_at(const Point& p,
  const Image& image,
  const CanvasGeo& geo,
  Bitmap& hitTestBuffer)
{
  // Create a small scaled and adjusted DC where the clicked pixel is
  // anchored at 0,0 in DC coordinates for pixel object hit tests
  FaintDC dc(hitTestBuffer);
  const coord zoom = geo.zoom.GetScaleFactor();
  dc.SetOrigin(-p * zoom);
  dc.SetScale(zoom);
  dc.Clear(mask_outside);

  // First consider the selected objects...
  Object* consider = 0;
  Hit considerType = Hit::NONE;
  const objects_t& objectSelection(image.GetObjectSelection());
  for (Object* object : top_to_bottom(objectSelection)){
    if (object->HitTest(p)){
      object->DrawMask(dc);
      Color color = dc.GetPixel(p);
      if (color == mask_edge){ // Fixme: Consider fuzzy or disabling AA
        return std::make_pair(object, Hit::BOUNDARY);
      }
      else if (color == mask_fill){
        return std::make_pair(object, Hit::INSIDE);
      }
      else {
        consider = object;
        considerType = Hit::NEARBY;
      }
    }
  }

  // ...then the rest
  dc.Clear(mask_outside);

  const objects_t& objects(image.GetObjects());
  for (Object* object : top_to_bottom(objects)){
    if (object->HitTest(p)){
      object->DrawMask(dc);
      Color color =  dc.GetPixel(p);
      if (color  == mask_edge){
        return std::make_pair(object, Hit::BOUNDARY);
      }
      else if (color == mask_fill){
        return std::make_pair(object, Hit::INSIDE);
      }
      else if ((consider == nullptr || considerType == Hit::NEARBY) &&
        color == mask_no_fill)
      {
        consider = object;
        considerType = Hit::INSIDE;
      }
      else if (consider == nullptr){
        consider = object;
        considerType = Hit::NEARBY;
      }
    }
  }
  return std::make_pair(consider, considerType);
}

template<typename T>
static int point_hit_test(const T& points,
  const IntPoint& ptView,
  const CanvasGeo& g,
  int objectHandleWidth)
{
  const Point imagePos(ptView.x - g.border.w + g.pos.x,
    ptView.y - g.border.h + g.pos.y);

  const Point handleOffset = Point::Both(objectHandleWidth / 2);
  const Size handleSize = Size::Both(objectHandleWidth);
  const coord zoom = g.zoom.GetScaleFactor();

  for (auto ptHandle : enumerate(points)){
    Point topLeft(*ptHandle * zoom - handleOffset);
    if (Rect(topLeft, handleSize).Contains(imagePos)){
      return ptHandle.num;
    }
  }
  return -1;
}

template<Handle H1, Handle H2, typename T>
static bool between(const Point& pos, const T& points, coord zoom){
  return side(pos, get<H1>(points) * zoom) !=
    side(pos, get<H2>(points) * zoom);
}

template<Handle H1, Handle H2, typename T>
Optional<Handle> find_nearest(const Point& pos,
  const T& points,
  coord zoom,
  coord& minDistance)
{
  coord dist = distance(pos, unbounded(get<H1>(points) * zoom));
  if (dist < minDistance){
    minDistance = dist;
    return option(H1);
  }
  dist = distance(pos, unbounded(get<H2>(points)*zoom));
  if (dist < minDistance){
    minDistance = dist;
    return option(H2);
  }
  return no_option();
}

template<typename T>
static Optional<Handle> get_side(const T& points, const IntPoint& ptView,
  const CanvasGeo& g)
{
  const Point pos(floated(ptView - point_from_size(g.border) + g.pos));
  coord zoom = g.Scale();
  coord minDistance = 5;
  Optional<Handle> nearest;

  if (between<Handle::P0P1, Handle::P2P3, T>(pos, points, zoom)){
    nearest |= find_nearest<Handle::P0P2, Handle::P1P3>(pos, points, zoom,
      minDistance);
  }
  if (between<Handle::P0P2, Handle::P1P3, T>(pos, points, zoom)){
    nearest |= find_nearest<Handle::P0P1, Handle::P2P3>(pos, points, zoom,
      minDistance);
  }
  return nearest;
}

template<typename T>
static Optional<Handle> get_handle(const T& points,
  const IntPoint& viewPos,
  const CanvasGeo& g,
  int handleWidth)
{
  int handleIndex = point_hit_test(points_number_order(points), viewPos, g,
    handleWidth);
  if (handleIndex != -1){
    return option(static_cast<Handle>(handleIndex));
  }

  // Not near a handle, check if near a side
  return get_side(points, viewPos, g);
}

ObjectInfo hit_test(const IntPoint& ptView,
  const Image& image,
  const CanvasGeo& geo,
  int objectHandleWidth)
{
  Bitmap hitTestBuffer(IntSize(1,1));

  Hit hitStatus = Hit::NONE;
  Object* object = 0;
  bool selected = false;
  HandleInfo handleInfo;
  // Selected objects are checked first so that they are prioritized
  const objects_t& objectSelection(image.GetObjectSelection());
  for (Object* objTemp : objectSelection){
    if (point_edit_enabled(objTemp)){
      int handleIndex = point_hit_test(objTemp->GetMovablePoints(),
        ptView, geo, objectHandleWidth);

      if (handleIndex != - 1){
        handleInfo.Set(EitherHandle({handleIndex, HandleType::MOVABLE_POINT}));
        hitStatus = Hit::BOUNDARY;
        selected = true;
        object = objTemp;
        break;
      }

      handleIndex = point_hit_test(objTemp->GetExtensionPoints(),
        ptView, geo, objectHandleWidth);
      if (handleIndex != -1){
        handleInfo.Set(EitherHandle(std::make_pair(handleIndex,
          HandleType::EXTENSION_POINT)));
        hitStatus = Hit::BOUNDARY;
        selected = true;
        object = objTemp;
        break;
      }
    }

    if (resize_handles_enabled(objTemp)){
      auto cornerPoints(object_aligned_resize(objTemp) ?
        objTemp->GetTri() :
        tri_from_rect(bounding_rect(objTemp->GetTri())));
      if (auto handle = get_handle(cornerPoints, ptView, geo, objectHandleWidth)){
        handleInfo.Set({handle.Get()});
        hitStatus = Hit::BOUNDARY;
        selected = true;
        object = objTemp;
        break;
      }
    }
  }

  if (handleInfo.NotSet()){
    Point ptImg = mouse::view_to_image(ptView, geo);
    std::pair<Object*, Hit> hitObject = object_at(ptImg, image, geo,
      hitTestBuffer);
    object = hitObject.first;
    hitStatus = hitObject.second;
    selected = (object == nullptr ? false :
      contains(image.GetObjectSelection(), object));
  }

  return ObjectInfo(object, hitStatus, selected, handleInfo);
}

} // namespace
