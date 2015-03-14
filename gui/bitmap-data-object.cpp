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

#include "wx/clipbrd.h"
#include "gui/bitmap-data-object.hh"

namespace{

struct bmp_info{
  int stride;
  int width;
  int height;
};

} // namespace

namespace faint {

FaintBitmapDataObject::FaintBitmapDataObject(const Bitmap& bmp)
  : wxDataObjectSimple(wxDataFormat("FaintBitmap")),
    m_bmp(bmp)
{}

FaintBitmapDataObject::FaintBitmapDataObject()
  : wxDataObjectSimple(wxDataFormat("FaintBitmap"))
{}

Bitmap FaintBitmapDataObject::GetBitmap() const{
  return m_bmp;
}

bool FaintBitmapDataObject::GetDataHere(void *buf) const{
  bmp_info info = {m_bmp.m_row_stride, m_bmp.m_w, m_bmp.m_h};
  memcpy(buf, &info, sizeof(bmp_info));
  memcpy(((char*)buf) + sizeof(bmp_info), m_bmp.m_data,
    to_size_t(m_bmp.m_h * m_bmp.m_row_stride));
  return true;
}

size_t FaintBitmapDataObject::GetDataSize() const{
  return sizeof(bmp_info) + to_size_t(m_bmp.m_h * m_bmp.m_row_stride);
}

bool FaintBitmapDataObject::SetData(size_t len, const void* buf){
  if (len < sizeof(bmp_info)){
    return false;
  }
  bmp_info info;
  memcpy(&info, buf, sizeof(bmp_info));
  if (len - sizeof(bmp_info) != (size_t)(info.height * info.stride)){
    return false;
  }

  m_bmp = Bitmap(IntSize(info.width, info.height), info.stride);
  memcpy(m_bmp.m_data, ((char*)buf) + sizeof(bmp_info), len - sizeof(bmp_info));
  return true;
}

bool FaintBitmapDataObject::SetData(const wxDataFormat&, size_t len,
  const void* buf)
{
  return SetData(len, buf);
}

} // namespace faint
