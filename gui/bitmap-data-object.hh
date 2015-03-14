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

#ifndef FAINT_BITMAP_DATA_OBJECT_HH
#define FAINT_BITMAP_DATA_OBJECT_HH
#include "wx/dataobj.h"
#include "bitmap/bitmap.hh"

namespace faint {

class FaintBitmapDataObject : public wxDataObjectSimple {
public:
  FaintBitmapDataObject();
  explicit FaintBitmapDataObject(const Bitmap&);
  Bitmap GetBitmap() const;
  bool GetDataHere(void* buf) const override;
  size_t GetDataSize() const override;
  bool SetData(size_t len, const void* buf) override;
  bool SetData(const wxDataFormat&, size_t len, const void*) override;

private:
  FaintBitmapDataObject(const FaintBitmapDataObject&);
  Bitmap m_bmp;
};

} // namespace

#endif
