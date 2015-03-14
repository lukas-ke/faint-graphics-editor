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

#include "gui/menu-predicate.hh"
#include "util/convenience.hh"

namespace faint{

  BoundMenuPred::BoundMenuPred(menu_item_id_t id, predicate_t predicate)
    : m_id(id),
      m_pred(predicate)
  {}

  void BoundMenuPred::Update(MenuReference& menu, const MenuFlags& flags) const{
    bool enable =
      (flags.toolSelection && fl(MenuPred::TOOL_SELECTION, m_pred)) ||
      (flags.rasterSelection && fl(MenuPred::RASTER_SELECTION, m_pred)) ||
      (flags.objectSelection && fl(MenuPred::OBJECT_SELECTION, m_pred)) ||
      (flags.hasObjects && fl(MenuPred::HAS_OBJECTS, m_pred)) ||
      (flags.dirty && fl(MenuPred::DIRTY, m_pred)) ||
      (flags.canUndo && fl(MenuPred::CAN_UNDO, m_pred)) ||
      (flags.canRedo && fl(MenuPred::CAN_REDO, m_pred)) ||
      (flags.numSelected > 1 && fl(MenuPred::MULTIPLE_SELECTED, m_pred)) ||
      (flags.groupIsSelected && fl(MenuPred::GROUP_IS_SELECTED, m_pred)) ||
      (flags.canMoveForward && fl(MenuPred::CAN_MOVE_FORWARD, m_pred)) ||
      (flags.canMoveBackward && fl(MenuPred::CAN_MOVE_BACKWARD, m_pred));

    menu.Enable(m_id, enable);
  }

  MenuPred::MenuPred(predicate_t predicate)
    : m_predicate(predicate)
  {}

  BoundMenuPred MenuPred::GetBound(menu_item_id_t id) const{
    return BoundMenuPred(id, m_predicate);
  }
}
