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

#ifndef FAINT_POS_INFO_HH
#define FAINT_POS_INFO_HH
#include "geo/calibration.hh"
#include "geo/object-handle.hh"
#include "geo/point.hh"
#include "text/utf8-string.hh"
#include "util/common-fwd.hh"
#include "util/either.hh"
#include "util/key-press.hh"
#include "util/pos-info-constants.hh"
#include "util/setting-id.hh"
#include "util/status-interface.hh"

namespace faint{

MouseButton the_other_one(MouseButton);

class ToolModifiers{
public:
  void SetLeftMouse();
  void SetRightMouse();
  void SetPrimary();
  void SetSecondary();
  bool LeftMouse() const;
  bool RightMouse() const;

  // Gets the featured mouse-button
  faint::MouseButton MouseButton() const;

  // True if the event features the specified button
  bool Feature(faint::MouseButton) const;
  bool Not(faint::MouseButton) const;

 // Normally ctrl
  bool Primary() const;

  // Normally shift
  bool Secondary() const;

  bool OnlyPrimary() const;
  bool OnlySecondary() const;
  bool Both() const;
  bool Either() const;
  bool Neither() const;

private:
  faint::MouseButton m_mouseButton = faint::MouseButton::NONE;
  bool m_primary = false;
  bool m_secondary = false;
};

// Either an index of a movable point or a resize-handle
using EitherHandle = Either<Handle,std::pair<object_handle_t, HandleType> >;

// Clicked handle of object, if any
using HandleInfo = Optional<EitherHandle>;

class ObjectInfo{
public:
  ObjectInfo(Object* object,
    Hit hitStatus,
    bool selected,
    const HandleInfo& handleInfo)
    : object(object),
      hitStatus(hitStatus),
      selected(selected),
      handleInfo(handleInfo)
  {}
  Object* object;
  Hit hitStatus;
  bool selected;
  HandleInfo handleInfo;
};

class PosInfo{
public:
  PosInfo(Canvas& canvas,
    StatusInterface& status,
    const ToolModifiers& modifiers,
    const Point& pos,
    int tabletCursor,
    bool inSelection,
    Layer layerType,
    ObjectInfo& objInfo)
    : canvas(canvas),
      status(status),
      modifiers(modifiers),
      pos(pos),
      tabletCursor(tabletCursor),
      inSelection(inSelection),
      layerType(layerType),
      hitStatus(objInfo.hitStatus),
      objSelected(objInfo.selected),
      object(objInfo.object),
      handle(objInfo.handleInfo)
  {}

  Canvas& canvas;
  StatusInterface& status;

  // Modifiers that were active when the event occured
  ToolModifiers modifiers;

  // The mouse position in image coordinates when the event occured
  Point pos;

  // A cursor id from a pen tablet
  int tabletCursor;

  // True if position is inside a raster selection
  bool inSelection;

  // The current layer-type choice
  Layer layerType;

  // Hitstatus of objects, relevant only if object layer
  Hit hitStatus;

  // True if object of hitStatus is a selected object.
  // Relevant only if object layer and hitStatus != HIT_NONE
  bool objSelected;

  // Hovered object (if any)
  Object* object;

  // Index of object:s clicked handle, or -1 if no object clicked
  HandleInfo handle;

  PosInfo& operator=(const PosInfo&) = delete;
};

class KeyInfo{
public:
  KeyInfo(Canvas& canvas,
    StatusInterface& status,
    Layer layerType,
    const KeyPress& key)
    : canvas(canvas),
      status(status),
      layerType(layerType),
      key(key)
  {}

  Canvas& canvas;
  StatusInterface& status;
  Layer layerType;
  KeyPress key;

  KeyInfo& operator=(const KeyInfo&) = delete;
};

class PosInside{
  // Wrapper for PosInfo denoting that the "pos" member is inside the
  // image.
  //
  // Access members in the wrapped PosInfo by ->.
  // The PosInside can also be converted implicitly to its contained
  // PosInfo.
public:
  operator const PosInfo&() const{
    return m_info;
  }

  const PosInfo* operator->(){
    return &m_info;
  }

  const PosInfo* operator->() const{
    return &m_info;
  }

  PosInside& operator=(const PosInside&) = delete;

private:
  explicit PosInside(const PosInfo& info) :
    m_info(info)
  {}
  friend Optional<PosInside> inside_canvas(const PosInfo&);

  const PosInfo& m_info;
};

Optional<PosInside> inside_canvas(const PosInfo&);
Optional<Calibration> get_calibration(const PosInfo&);
Optional<Calibration> get_calibration(const KeyInfo&);

ExpressionContext& get_expression_context(const PosInfo&);

} // namespace

#endif
