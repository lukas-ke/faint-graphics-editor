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

#ifndef FAINT_BITMAP_LIST_CTRL_HH
#define FAINT_BITMAP_LIST_CTRL_HH
#include <functional>
#include <vector>
#include "wx/panel.h"
#include "geo/axis.hh"
#include "geo/int-size.hh"
#include "text/utf8-string.hh"
#include "util/index.hh"

namespace faint{

class StatusInterface;

class BitmapListCtrl : public wxPanel {
  // A vertical or horizontal list box choice control with icons
  // indicating the choices
  // Example: <../doc/bitmap-list-ctrl.png>
public:
  // Fixme: Rework this to take a list of bitmaps + status-texts, remove Add
  BitmapListCtrl(wxWindow* parent,
    const IntSize& imageSize,
    StatusInterface&,
    Axis orientation=Axis::VERTICAL,
    IntSize spacing={5,5});

  Index Add(const wxBitmap&, const utf8_string& statusText);
  Index GetSelection() const;
  void SetSelection(const Index&);

private:
  std::vector<wxBitmap> m_images;
  const IntSize m_imageSize;
  const Axis m_orientation;
  Index m_selection;
  const IntSize m_spacing;
  StatusInterface& m_status;
  std::vector<utf8_string> m_statusTexts;
};

} // namespace

namespace faint{ namespace events{

void on_selection(BitmapListCtrl*,
  const std::function<void(const Index&)>&);

}} // namespace

#endif
