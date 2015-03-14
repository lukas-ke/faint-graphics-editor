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
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "gui/bitmap-data-object.hh"
#include "gui/object-data-object.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/convert-wx.hh"

namespace faint {

Clipboard::Clipboard(){
  m_ok = wxTheClipboard->Open();
}

Clipboard::~Clipboard(){
  if (m_ok){
    wxTheClipboard->Close();
  }
}

void Clipboard::Flush(){
  if (wxTheClipboard->Open()){
    wxTheClipboard->Flush();
    wxTheClipboard->Close();
  }
}

Optional<Bitmap> Clipboard::GetBitmap(){
  assert(m_ok);
  wxDataObjectComposite composite;
  FaintBitmapDataObject* bmpObject_faint = new FaintBitmapDataObject;
  wxBitmapDataObject* bmpObject_wx = new wxBitmapDataObject;
  composite.Add(bmpObject_faint, true);
  composite.Add(bmpObject_wx, false);

  if (wxTheClipboard->GetData(composite)){
    wxDataFormat format = composite.GetReceivedFormat();
    if (format == wxDataFormat("FaintBitmap")){
      return option(bmpObject_faint->GetBitmap());
    }
    else{
      wxBitmap wxBmp = bmpObject_wx->GetBitmap();
      return option(to_faint(clean_bitmap(wxBmp)));
    }
  }
  return no_option();
}

Optional<objects_t> Clipboard::GetObjects(){
  assert(m_ok);

  ObjectDataObject objectData;
  if (!wxTheClipboard->GetData(objectData)){
    return no_option();
  }
  return option(objectData.GetObjects());
}

Optional<utf8_string> Clipboard::GetText(){
  assert(m_ok);

  wxTextDataObject textObject;
  if (!wxTheClipboard->GetData(textObject)){
    return no_option();
  }
  return option(to_faint(textObject.GetText()));
}

bool Clipboard::Good() const{
  return m_ok;
}

void Clipboard::SetBitmap(const Bitmap& bmp, const ColRGB& bgCol){
  assert(m_ok);
  wxDataObjectComposite* composite = new wxDataObjectComposite;
  composite->Add(new FaintBitmapDataObject(bmp), true);
  composite->Add(new wxBitmapDataObject(to_wx_bmp(alpha_blended(bmp, bgCol))));
  wxTheClipboard->SetData(composite);
}

void Clipboard::SetObjects(const objects_t& objects){
  assert(m_ok);
  wxTheClipboard->SetData(new ObjectDataObject(objects));
}

void Clipboard::SetText(const utf8_string& text){
  assert(m_ok);
  wxTheClipboard->SetData(new wxTextDataObject(to_wx(text)));
}

} // Namespace
