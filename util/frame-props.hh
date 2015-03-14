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

#ifndef FAINT_FRAME_PROPS_HH
#define FAINT_FRAME_PROPS_HH
#include "bitmap/bitmap-fwd.hh"
#include "geo/calibration.hh"
#include "geo/geo-fwd.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "util/color-span.hh"
#include "util/delay.hh"
#include "util/distinct.hh"
#include "util/either.hh"
#include "util/hot-spot.hh"
#include "util/index.hh"
#include "util/objects.hh"

namespace faint{

class Bitmap;

class category_imageinfo;
using create_bitmap = Distinct<bool, category_imageinfo, 0>;

class ImageInfo {
public:
  explicit ImageInfo(const IntSize&, const create_bitmap&);
  ImageInfo(const IntSize&, const Color& bg, const create_bitmap&);
  IntSize size;
  Color backgroundColor;
  bool createBitmap;
};

class FrameInfo {
  // Properties for a single frame
public:
  explicit FrameInfo();
  explicit FrameInfo(const Delay&);
  explicit FrameInfo(const HotSpot&);
  FrameInfo(const Delay&, const HotSpot&);
  Delay delay;
  HotSpot hotSpot;
};

class FrameProps{
public:
  FrameProps();
  FrameProps(FrameProps&&);
  FrameProps(const FrameProps&) = delete;
  explicit FrameProps(const Bitmap&);
  explicit FrameProps(const ImageInfo&);
  FrameProps(const Bitmap&, const FrameInfo&);
  FrameProps(const Bitmap&, const objects_t&);
  FrameProps(const IntSize&, const objects_t&);
  ~FrameProps();
  Index AddObject(Object*);
  const Either<Bitmap, ColorSpan>& GetBackground() const;
  const Optional<Calibration>& GetCalibration() const;
  Delay GetDelay() const;
  HotSpot GetHotSpot() const;
  Object* GetObject(const Index&);
  bool HasObject(const Index&) const;
  bool IsTopLevel(const Index&) const;
  void RemoveObject(Object*);
  void SetBackground(const Either<Bitmap, ColorSpan>&);
  void SetCalibration(const Calibration&);
  objects_t TakeObjects();
private:
  objects_t m_allObjects;
  Either<Bitmap, ColorSpan> m_background;
  Optional<Calibration> m_calibration;
  Delay m_delay;
  HotSpot m_hotSpot;
  objects_t m_objects;
};

} // namespace

#endif
