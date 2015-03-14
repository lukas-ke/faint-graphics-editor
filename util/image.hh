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

#ifndef FAINT_IMAGE_HH
#define FAINT_IMAGE_HH
#include <vector>
#include "bitmap/bitmap.hh"
#include "geo/calibration.hh"
#include "geo/geo-fwd.hh"
#include "util/color-span.hh"
#include "util/delay.hh"
#include "util/either.hh"
#include "util/hot-spot.hh"
#include "util/id-types.hh"
#include "util/optional.hh"
#include "util/raster-selection.hh"

namespace faint {

class ExpressionContext;
class FrameProps;
class Object;

using objects_t = std::vector<Object*>;

class Image {
public:
  Image(FrameProps&&);
  Image(const Image& other);
  Image();
  ~Image();
  void Add(Object*);
  void Add(Object*, int z);

  // Converts a ColorSpan background to a bitmap, and returns the
  // created Bitmap. Stores the ColorSpan for Revert(). Must not be
  // called if the background isn't a ColorSpan.
  Bitmap& ConvertColorSpanToBitmap();

  bool Deselect(const Object*);
  bool Deselect(const objects_t&);
  void DeselectObjects();
  const Either<Bitmap, ColorSpan>& GetBackground() const;
  FrozenEither<Bitmap, ColorSpan> GetBackground();

  // The time to remain on this image if saved as an animation.
  Delay GetDelay() const;

  ExpressionContext& GetExpressionContext() const;
  HotSpot GetHotSpot() const;
  FrameId GetId() const;
  int GetNumObjects() const;

  const objects_t& GetObjects() const;
  int GetObjectZ(const Object*) const;
  const objects_t& GetObjectSelection() const;
  RasterSelection& GetRasterSelection();
  const Optional<Calibration>& GetCalibration() const;
  const RasterSelection& GetRasterSelection() const;
  IntSize GetSize() const;
  bool Has(const ObjectId&) const;
  bool Has(const Object*) const;
  bool HasStoredOriginal() const;
  void Remove(Object*);
  void Revert();

  // Select the specified objects, optionally deselecting
  // the current selection.
  // The objects must exist in this image.
  void SelectObjects(const objects_t&);
  void SetBitmap(const Bitmap&);
  void SetBitmap(Bitmap&&);
  void SetCalibration(const Optional<Calibration>&);
  void SetDelay(const Delay&);
  void SetHotSpot(const HotSpot&);
  void SetObjectZ(Object*, int z);

  // Stores the current state as the original image for Revert(). This
  // is used when undoing, the image is reverted and all raster
  // commands reapplied. StoreAsOriginal() must have been called before
  // undo is used.
  //
  // This is not done on construction because it would use a lot of
  // memory when opening many images, and should instead be called
  // before applying the first modification (command).
  //
  // Must not be called if HasStoredOriginal().
  void StoreAsOriginal();

  Image& operator=(const Image&) = delete;
private:
  Either<Bitmap, ColorSpan> m_bg;
  Optional<Calibration> m_calibration;
  Delay m_delay;
  ExpressionContext* m_expressionContext;
  HotSpot m_hotSpot;
  FrameId m_id;
  objects_t m_objects;
  objects_t m_objectSelection;
  Optional<Either<Bitmap, ColorSpan> > m_original;
  objects_t m_originalObjects;
  RasterSelection m_rasterSelection;
};

// Rectangle with the same size as the image, anchored at 0,0
IntRect image_rect(const Image&);

} // namespace faint

#endif
