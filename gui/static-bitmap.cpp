// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "wx/dcbuffer.h" // wxAutoBufferedPaintDC
#include "bitmap/bitmap.hh"
#include "gui/static-bitmap.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

StaticBitmap::StaticBitmap(wxWindow* parent, const Bitmap& bmp)
  : wxPanel(parent, wxID_ANY),
    m_bmp(to_wx_bmp(bmp))
{
  SetBackgroundStyle(wxBG_STYLE_PAINT); // Prevent flicker on full refresh
  SetInitialSize(m_bmp.GetSize());

  events::on_paint(this, [this](){
    wxAutoBufferedPaintDC dc(this);
    dc.DrawBitmap(m_bmp, 0, 0);
  });
}

void StaticBitmap::SetBitmap(const Bitmap& bmp){
  m_bmp = to_wx_bmp(bmp);
  Refresh();
}

} // namespace
