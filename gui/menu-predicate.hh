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

#ifndef FAINT_MENU_PREDICATE_HH
#define FAINT_MENU_PREDICATE_HH
#include "text/utf8-string.hh"

namespace faint{

struct MenuFlags{
  bool toolSelection;
  bool rasterSelection;
  bool objectSelection;
  size_t numSelected;
  bool groupIsSelected;
  bool hasObjects;
  bool dirty;
  bool canUndo;
  bool canRedo;
  bool canMoveForward;
  bool canMoveBackward;
  utf8_string undoLabel;
  utf8_string redoLabel;
};

using menu_item_id_t = int;
using predicate_t =  int;

class MenuReference{
  // Hides the concrete menu implementation from BoundMenuPred (to
  // avoid explicit dependency).
public:
  virtual ~MenuReference() = default;

  // Enable or disable a menu item
  virtual void Enable(menu_item_id_t, bool enable) = 0;
};

class BoundMenuPred{
  // A condition tied to a specific menu item.
public:
  BoundMenuPred(menu_item_id_t, predicate_t);
  void Update(MenuReference&, const MenuFlags&) const;
private:
  menu_item_id_t m_id;
  predicate_t m_pred;
};

class MenuPred{
  // A condition that must be filled for menu items to be enabled.
  // A MenuPred is just the condition, to get a predicate that is
  // connected to a menu item use GetBound(...).
  //
  // The distinction allows more flexible use, for example for
  // passing passing the id and a predicate to a function without
  // having to duplicate the id like this:
  // > add_menu_item_ugly(id, BoundMenuPred(id, HAS_OBJECTS));
  // Nicer (avoids redundant id):
  // > add_menu_item_nice(id, MenuPred(HAS_OBJECTS))
  // An alternative:
  // > add_menu_item_unintuitive(BoundMenuPred(id, HAS_OBJECTS));
  // would suggest that the predicate is a requirement, and obscure
  // the role of the id.
  // This also allows a single MenuPred to be applied to multiple
  // menu_item_id:s.
public:
  enum{
    // Conditions for enabling a menu item.
    TOOL_SELECTION=1, // the active tool has some selection mechanism
    OBJECT_SELECTION=2, // objects are selected, and indicated as such
    RASTER_SELECTION=4, // a raster area is selected and indicated
    HAS_OBJECTS=8, // the image contains objects
    CAN_UNDO=16,
    CAN_REDO=32,
    DIRTY=64,
    MULTIPLE_SELECTED=128,
    GROUP_IS_SELECTED=256,
    CAN_MOVE_FORWARD=512,
    CAN_MOVE_BACKWARD=1024,
    ANY_SELECTION=TOOL_SELECTION | OBJECT_SELECTION | RASTER_SELECTION };
  MenuPred(predicate_t);
  BoundMenuPred GetBound(menu_item_id_t) const;
private:
  predicate_t m_predicate;
};

} // namespace

#endif
