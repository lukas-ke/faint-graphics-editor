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

#include "wx/dnd.h"
#include "gui/color-data-object.hh"
#include "gui/drop-source.hh"

namespace faint{

FaintDropSource::FaintDropSource(wxWindow* window, ColorDataObject& object)
  : wxImpl(window),
    m_object(object)
{
  wxImpl.SetData(object);
}

wxDragResult FaintDropSource::CustomDoDragDrop(){
  wxImpl.DoDragDrop(wxDrag_AllowMove);
  return m_object.GetOperationType();
}

} // namespace
