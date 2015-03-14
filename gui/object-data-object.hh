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

#ifndef FAINT_OBJECT_DATA_OBJECT_HH
#define FAINT_OBJECT_DATA_OBJECT_HH
#include "wx/dataobj.h"
#include "util/objects.hh"

namespace faint{

class ObjectDataObject : public wxDataObject {
  // Clipboard support for Faint objects
public:
  ObjectDataObject();
  explicit ObjectDataObject(const objects_t&);
  ~ObjectDataObject();
  void GetAllFormats(wxDataFormat*, Direction) const override;
  size_t GetDataSize(const wxDataFormat&) const override;
  bool GetDataHere(const wxDataFormat&, void* buf) const override;
  size_t GetFormatCount(Direction) const override;
  wxDataFormat GetPreferredFormat(Direction) const override;
  bool SetData(const wxDataFormat&, size_t len, const void* buf) override;

  // Note: Calling code is responsible for deleting the returned
  // objects
  objects_t GetObjects() const;
private:
  objects_t m_objects;
};

} // namespace

#endif
