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

#include "wx/window.h"
#include "geo/int-point.hh"
#include "gui/color-data-object.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"

namespace faint{

wxDragResult ColorDataObject::m_dragResult = wxDragNone;

struct ColorStruct{
  int r;
  int g;
  int b;
  int a;
};

ColorDataObject::ColorDataObject(const Color& color)
  : m_color(color)
{
  m_dragResult = wxDragNone;
}

void ColorDataObject::GetAllFormats(wxDataFormat* formats, Direction) const{
  formats[0] = wxDataFormat("FaintColor");
  formats[1] = wxDataFormat(wxDF_TEXT);
}

Color ColorDataObject::GetColor() const{
  return m_color;
}

bool ColorDataObject::GetDataHere(const wxDataFormat& format, void *buf) const{
  if (format == wxDataFormat("FaintColor")){
    ColorStruct c = {m_color.r, m_color.g, m_color.b, m_color.a };
    memcpy(buf, &c, sizeof(c));
    return true;
  }
  else if (format == wxDataFormat(wxDF_TEXT)){
    wxString dragText(to_wx(str_smart_rgba(m_color)));
    wxTextDataObject textObj(dragText);
    textObj.GetDataHere(format, buf);
    return true;
  }
  return false;
}

size_t ColorDataObject::GetDataSize(const wxDataFormat& f) const{
  if (f == wxDataFormat("FaintColor")){
    return sizeof(ColorStruct);
  }
  else if (f == wxDataFormat(wxDF_TEXT)){
    wxString dragText(to_wx(str_smart_rgba(m_color)));
    wxTextDataObject textObj(dragText);
    return textObj.GetDataSize();
  }
  return 0;
}

size_t ColorDataObject::GetFormatCount(Direction) const{
  return 2;
}

wxDragResult ColorDataObject::GetOperationType() const{
  return m_dragResult;
}

wxDataFormat ColorDataObject::GetPreferredFormat(Direction) const {
  return wxDataFormat("FaintColor");
}

bool ColorDataObject::SetData(const wxDataFormat& format, size_t len,
  const void* buf)
{
  if (format == wxDataFormat("FaintColor") && len == sizeof(ColorStruct)){
    ColorStruct c;
    memcpy(&c, buf, len);
    m_color = color_from_ints(c.r, c.g, c.b, c.a);
    return true;
  }
  return false;
}

void ColorDataObject::SetOperationType(wxDragResult result){
  m_dragResult = result;
}

class ColorDropTargetImpl : public wxDropTarget {
  // Impl-class to  hide the large wxDropTarget interface
public:
  ColorDropTargetImpl(ColorDropTarget* target);
  bool OnDrop(wxCoord x, wxCoord y);
  virtual wxDragResult OnData(wxCoord x, wxCoord y, wxDragResult def);

private:
  ColorDataObject* colorObj;
  ColorDropTarget* m_target;
};

ColorDropTargetImpl::ColorDropTargetImpl(ColorDropTarget* targetWindow){
  m_target = targetWindow;
  colorObj = new ColorDataObject(Color(0,0,0));
  SetDataObject(colorObj);
}

wxDragResult ColorDropTargetImpl::OnData(wxCoord x, wxCoord y, wxDragResult){
  GetData(); // Copy the data into our data object
  wxDragResult result = m_target->OnDropColor(IntPoint(x, y),
    colorObj->GetColor());
  colorObj->SetOperationType(result);
  return result;
}

bool ColorDropTargetImpl::OnDrop(wxCoord, wxCoord){
  return true; // Allow drop for the entire window
}

ColorDropTarget::ColorDropTarget(wxWindow* targetWindow){
  // AFAIK, deletion of impl is handled by wxWidgets.
  ColorDropTargetImpl* impl = new ColorDropTargetImpl(this);
  targetWindow->SetDropTarget(impl);
}

} // namespace
