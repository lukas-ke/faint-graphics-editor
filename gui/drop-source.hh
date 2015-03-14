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

#ifndef FAINT_DROP_SOURCE_HH
#define FAINT_DROP_SOURCE_HH
class wxWindow;
class ColorDataObject;
class wxDropSource;

namespace faint{

class FaintDropSource {
 public:
  FaintDropSource(wxWindow*, ColorDataObject&);

  // Performs the drag and drop and returns whether the result was a
  // move or copy operation, based on the static result stored in the
  // data object (This was the only way I could figure out to base the
  // operation on other things than keypresses)
  wxDragResult CustomDoDragDrop();
 private:
  wxDropSource wxImpl;
  ColorDataObject& m_object;
};

} // namespace

#endif
