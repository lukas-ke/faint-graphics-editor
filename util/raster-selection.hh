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

#ifndef FAINT_RASTER_SELECTION_HH
#define FAINT_RASTER_SELECTION_HH
#include "bitmap/bitmap.hh"
#include "bitmap/paint.hh"
#include "geo/int-rect.hh"
#include "util/distinct.hh"

namespace faint{

class FaintDC;
class Overlays;
class Settings;

class category_raster_selection;
using copy_selected = Distinct<bool, category_raster_selection, 0>;

class SelectionOptions{
public:
  SelectionOptions(bool mask, const Paint& bg, bool alpha);
  bool alpha;
  Paint bg;
  bool mask;
};

using NewSelectionOptions = Order<SelectionOptions>::New;
using OldSelectionOptions = Order<SelectionOptions>::Old;

class SelectionState{
public:
  // No selection
  SelectionState()
    : copy(false),
      floating(false)
  {}
  // A floating pasted bitmap
  SelectionState(const Bitmap&, const IntPoint&);

  // A non-floating rectangle selection
  SelectionState(const IntRect&);

  // Non-pasted float (leaves a hole)
  SelectionState(const Bitmap&, const IntPoint&, const IntRect& oldRect);

  bool Floating() const;
private:
  SelectionState& operator=(const SelectionState&);
  friend class RasterSelection;
  bool copy;
  bool floating;
  Bitmap floatingBmp;
  IntRect oldRect;
  IntRect rect;
};

using NewSelectionState = Order<SelectionState>::New;
using OldSelectionState = Order<SelectionState>::Old;

class RasterSelection;

namespace sel{
  class Empty{
  };

  class Existing{
  public:
    explicit Existing(const RasterSelection&);
    const RasterSelection& Get() const;
    const SelectionState& GetState() const;
    Existing& operator=(const Existing&) = delete;
  private:
    const RasterSelection& m_selection;
  };

  class Rectangle{
  public:
    Rectangle(const IntRect&, const SelectionOptions&);
    virtual ~Rectangle() = default;
    IntPoint TopLeft() const;
    IntRect Rect() const;
    Rectangle& operator=(const Rectangle&) = delete;
    const SelectionOptions& GetOptions() const;
  private:
    SelectionOptions m_options;
    const IntRect m_rect;
  };

  class Floating : public Rectangle{
  public:
    Floating(const IntRect&, const Bitmap&, const SelectionOptions&);
    const Bitmap& GetBitmap() const;
  private:
    const Bitmap& m_bitmap;
  };

  class Moving : public Floating{
  public:
    explicit Moving(const IntRect&, const IntRect&, const Bitmap&,
      const SelectionOptions&);
    IntRect OldRect() const;
  private:
    const IntRect m_oldRect;
  };

  class Copying : public Floating{
  public:
    explicit Copying(const IntRect&, const Bitmap&, const SelectionOptions&);
  };
} // namespace

class RasterSelection{
  // A selected region in the image or a floating selection.
  // A  selected region indicates a region on the image, while a
  // floating floating selection is a movable overlay graphic,
  // for example pasted raster graphics.
  //
  // A floating selection can be moved in steps without
  // affecting the image (until the selection is committed).
  // A non-floating selection can become floating, which
  // will leave a hole behind
public:
  friend class SelectionState;
  RasterSelection();
  // Changes the current selection to a floating selection.
  // Must not be Empty().
  void BeginFloat(const Bitmap& src, const copy_selected&);

  // Clips a non-floating selection to the specified rectangle
  // Does nothing for floating selections.
  void Clip(const IntRect&);

  bool Copying() const;

  // True if the selection exists and contains the point.
  bool Contains(const IntPoint&);

  // Removes any selection, loses any floating image
  void Deselect();

  // Draws the floating image (if any), does not draw
  // any selection indicators.
  void DrawFloating(FaintDC&) const;

  // Adds the selection outline to the overlays
  void DrawOutline(Overlays&) const;

  // True if there's no selected region or floating selection
  bool Empty() const;

  // Opposite of Empty()
  bool Exists() const;
  bool Floating() const;

  const Paint& GetBackground() const;

  // Returns the floating bitmap. Note: Must be Floating()
  const Bitmap& GetBitmap() const;
  IntRect GetOldRect() const;
  SelectionOptions GetOptions() const;

  // Note: Must not be Empty()
  IntRect GetRect() const;
  const SelectionState& GetState() const;
  IntSize GetSize() const;
  void Move(const IntPoint& topLeft);

  // Offsets the selection and the erased background (if any)
  void OffsetOrigin(const IntPoint& delta);

  // Creates a floating selection from "outside"
  void Paste(const Bitmap&, const IntPoint& topLeft);
  void SetAlphaBlending(bool);
  void SetBackground(const Paint&);

  // Changes the floating bitmap.
  // Must be Floating(), unlike Paste(...)
  void SetFloatingBitmap(const Bitmap&, const IntPoint& topLeft);
  void SetMask(bool);

  void SetOptions(const SelectionOptions&);

  // Specifies the non-floating selection rectangle.
  // Must not be called if Floating().
  void SetRect(const IntRect&);

  void SetState(const SelectionState&);

  IntPoint TopLeft() const;
private:
  SelectionState m_state;
  SelectionOptions m_options;
};

void update_mask(bool enableMask,
  const Paint& bg,
  bool enableAlpha,
  RasterSelection&);

SelectionOptions raster_selection_options(const Settings&);

} // namespace

#endif
