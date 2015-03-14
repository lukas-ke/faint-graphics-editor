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

#ifndef FAINT_STATIC_BITMAP_HH
#define FAINT_STATIC_BITMAP_HH
#include "wx/bitmap.h"
#include "wx/panel.h"

namespace faint {

class Bitmap;

class StaticBitmap : public wxPanel{
  // Like wxStaticBitmap but for faint::Bitmaps, and
  // without flicker on Windows.
public:
  StaticBitmap(wxWindow* parent, const Bitmap&);
  void SetBitmap(const Bitmap&);

private:
  wxBitmap m_bmp;
};

} // namespace

#endif
