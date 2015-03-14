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

#include "wx/bitmap.h"
#include "wx/defs.h"
#include "wx/event.h"
#include "wx/clipbrd.h"
#include "wx/filename.h"
#include "wx/rawbmp.h"
#include "wx/tokenzr.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "gui/art-container.hh"
#include "text/utf8-string.hh"
#include "util/pos-info.hh"
#include "util-wx/file-path.hh"

#ifdef __WXMSW__
#include "wx/dcmemory.h"
#endif

namespace faint{

wxFileName absoluted(const wxFileName& filename){
  wxFileName other(filename);
  other.MakeAbsolute();
  return other;
}

wxColour to_wx(const ColRGB& c){
  return wxColour(c.r, c.g, c.b);
}

wxColour to_wx(const Color& c){
  return wxColour(c.r, c.g, c.b, c.a);
}

wxRect to_wx(const IntRect& r){
  return wxRect(r.x, r.y, r.w, r.h);
}

wxPoint to_wx(const IntPoint& p){
  return wxPoint(p.x, p.y);
}

wxSize to_wx(const IntSize& sz){
  return wxSize(sz.w, sz.h);
}

IntPoint to_faint(const wxPoint& p){
  return IntPoint(p.x, p.y);
}

Color to_faint(const wxColour& c){
  return Color(c.Red(), c.Green(), c.Blue(), c.Alpha());
}

IntRect to_faint(const wxRect& r){
  return IntRect(IntPoint(r.x, r.y), IntSize(r.width, r.height));
}

IntSize to_faint(const wxSize& sz){
  return IntSize(sz.GetWidth(), sz.GetHeight());
}

static ToolModifiers get_key_tool_modifiers(){
  ToolModifiers modifiers;
  if (wxGetKeyState(WXK_CONTROL)){
    modifiers.SetPrimary();
  }
  if (wxGetKeyState(WXK_SHIFT)){
    modifiers.SetSecondary();
  }
  return modifiers;
}

ToolModifiers get_tool_modifiers(){
  ToolModifiers modifiers = get_key_tool_modifiers();
  wxMouseState mouseState = wxGetMouseState();
  if (mouseState.LeftIsDown()){
    modifiers.SetLeftMouse();
  }
  else if (mouseState.RightIsDown()){
    modifiers.SetRightMouse();
  }
  return modifiers;
}

ToolModifiers mouse_modifiers(const wxMouseEvent& event){
  ToolModifiers modifiers = get_key_tool_modifiers();
  if (event.LeftUp() || event.LeftDown() || event.LeftDClick()){
    modifiers.SetLeftMouse();
  }
  else if (event.RightUp() || event.RightDown()){
    modifiers.SetRightMouse();
  }
  return modifiers;
}

Mod key_modifiers(const wxKeyEvent& event){
  return Ctrl.If(event.ControlDown()) +
    Shift.If(event.ShiftDown()) +
    Alt.If(event.AltDown());
}

wxImage to_wx_image(const Bitmap& bmp){
  const int stride = bmp.m_row_stride;
  const uchar* bgraData = bmp.m_data;

  // Using malloc to match wxWidgets free
  uchar* rgbData = (uchar*)malloc(to_size_t(bmp.m_w * bmp.m_h * 3));
  uchar* aData = (uchar*)malloc(to_size_t(bmp.m_w * bmp.m_h));

  for (int y = 0; y != bmp.m_h; y++){
    for (int x = 0; x != bmp.m_w; x++){
      size_t srcPos = to_size_t(y * stride + x * BPP);
      size_t dstPos_rgb = to_size_t(y * bmp.m_w * 3 + x * 3);
      size_t dstPos_alpha = to_size_t(y * bmp.m_w + x);
      rgbData[dstPos_rgb] = bgraData[srcPos + iR];
      rgbData[dstPos_rgb + 1] = bgraData[srcPos + iG];
      rgbData[dstPos_rgb + 2] = bgraData[srcPos + iB];
      aData[dstPos_alpha] = bgraData[srcPos + iA];
    }
  }

  // wxWidgets takes ownership of rgbData and aData, and uses free()
  // to release the memory.
  wxImage result(bmp.m_w, bmp.m_h, rgbData, aData);
  return result;
}

using AlphaPixelData = wxPixelData<wxBitmap, wxAlphaPixelFormat>;
using PixelData = AlphaPixelData;
wxBitmap to_wx_bmp(const Bitmap& bmp){
  wxBitmap wxBmp(bmp.m_w, bmp.m_h, 32);
  PixelData pData(wxBmp);
  assert(pData);
  PixelData::Iterator p = pData;

  uchar* data = bmp.m_data;
  const int stride = bmp.m_row_stride;

  for (int y = 0; y != bmp.m_h; y++){
    PixelData::Iterator rowStart = p;
    for (int x = 0; x != bmp.m_w; x++){
      int pos = y *  stride + x * BPP;
      #ifdef __WXMSW__
      const uchar alpha = *(data + pos + iA);
      p.Alpha() = alpha;
      // Todo: What. Simplify.
      p.Red() = (*(data + pos + iR) * (255 - (255 - alpha))) / 255;
      p.Green() = (*(data + pos + iG) * (255 - (255 - alpha))) / 255;
      p.Blue() = (*(data + pos + iB) * (255 - (255 - alpha))) / 255;
      #else
      p.Alpha() = *(data + pos + iA);
      p.Red()   = *(data + pos + iR);
      p.Green() = *(data + pos + iG);
      p.Blue()  = *(data + pos + iB);
      #endif
      ++p;
    }
    p = rowStart;
    p.OffsetY(pData, 1);
  }
  return wxBmp;
}

Bitmap to_faint(wxBitmap wxBmp){
  Bitmap bmp(to_faint(wxBmp.GetSize()));
  if (wxBmp.GetDepth() == 24){
    wxNativePixelData pixelData(wxBmp);
    if (!pixelData){
      goto alpha_label;
    }
    wxNativePixelData::Iterator p = pixelData;
    const int stride = bmp.m_row_stride;
    uchar* data = bmp.GetRaw();
    for (int y = 0; y != bmp.m_h; y++){
      wxPixelData<wxBitmap, wxNativePixelFormat>::Iterator rowStart = p;
      for (int x = 0; x != bmp.m_w; x++){
        size_t pos = to_size_t(y * stride + x * BPP);
        data[pos + iA] = 255;
        data[pos + iR] = p.Red();
        data[pos + iG] = p.Green();
        data[pos + iB] = p.Blue();
        ++p;
      }
      p = rowStart;
      p.OffsetY(pixelData, 1);
    }
  }
  else {
  alpha_label:
    AlphaPixelData pixelData(wxBmp);
    assert(pixelData);
    AlphaPixelData::Iterator p = pixelData;
    const int stride = bmp.m_row_stride;
    uchar* data = bmp.GetRaw();
    for (int y = 0; y != bmp.m_h; y++){
      AlphaPixelData::Iterator rowStart = p;
      for (int x = 0; x != bmp.m_w; x++){
        size_t pos = to_size_t(y * stride + x * BPP);
        #ifdef __WXMSW__
        // Convert back from premultiplied alpha
        data[pos + iA] = p.Alpha();
        if (p.Alpha() != 0){
          data[pos + iR] = (p.Red() * 255) / (255 - (255 - p.Alpha()));
          data[pos + iG] = (p.Green() * 255) / (255 - (255 - p.Alpha()));
          data[pos + iB] = (p.Blue() * 255) / (255 - (255 - p.Alpha()));
        }
        #else
        data[pos + iA] = p.Alpha();
        data[pos + iR] = p.Red();
        data[pos + iG] = p.Green();
        data[pos + iB] = p.Blue();
        #endif
        ++p;
      }
      p = rowStart;
      p.OffsetY(pixelData, 1);
    }
  }
  return bmp;
}

#ifdef __WXMSW__
// Without this, pasting from clipboard in MSW causes this error:
// msw\bitmap.cpp(1287): assert "Assert failure" failed in
// wxBitmap::GetRawData(): failed to get DIBSECTION from a DIB?
//
// Probably this bug: http://trac.wxwidgets.org/ticket/11640
wxBitmap clean_bitmap(const wxBitmap& dirtyBmp){
  // 24bpp depth loses alpha information, but wxMemoryDC does not
  // support alpha Setting this to 32 or retrieving from the bitmap
  // works for pastes between applications and within Faint, but
  // fails for pastes from print screen (gives very weird effects,
  // probably random alpha values).
  // I've also tried GCDC Blit and DrawBitmap, neither retained alpha
  wxBitmap cleanBmp(dirtyBmp.GetWidth(), dirtyBmp.GetHeight(), 24);
  wxMemoryDC cleanDC(cleanBmp);
  cleanDC.DrawBitmap(dirtyBmp, 0, 0);
  cleanDC.SelectObject(wxNullBitmap);
  return cleanBmp;
}
#endif

#ifndef __WXMSW__

wxBitmap clean_bitmap(const wxBitmap& dirtyBmp){
  return dirtyBmp;
}

#endif

utf8_char to_faint(const wxChar& ch){
  wxString s;
  s += ch;
  wxCharBuffer buf = s.utf8_str();
  return utf8_char(std::string(buf, buf.length()));
}

utf8_string to_faint(const wxString& str){
  wxCharBuffer buf(str.utf8_str());
  return utf8_string(std::string(buf, buf.length()));
}

wxString to_wx(const utf8_string& faintStr){
  return wxString::FromUTF8(faintStr.c_str());
}

FileList to_FileList(const wxArrayString& strings){
  FileList files;
  for (const wxString& str : strings){
    files.push_back(FilePath::FromAbsoluteWx(str));
  }
  return files;
}

wxString get_clipboard_text(){
  wxTextDataObject textObject;
  if (!wxTheClipboard->GetData(textObject)){
    return "";
  }
  return textObject.GetText();
}

std::vector<wxString> wx_split_lines(const wxString& text){
  wxStringTokenizer tokenizer(text, "\r\n");
  std::vector<wxString> v;
  while (tokenizer.HasMoreTokens()){
    v.push_back(tokenizer.GetNextToken());
  }
  return v;
}


#ifdef FAINT_MSW
std::wstring iostream_friendly(const wxString& path){
  // As best I can tell this wxWidgets method will provide an
  // UTF16-encoded std::wstring, which the Visual C++-implementation
  // of iostream accepts.
  return path.ToStdWstring();
}
#else
std::string iostream_friendly(const wxString& path){
  return to_faint(path).str();
}
#endif

} // namespace
