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

#include "wx/popupwin.h"
#include "wx/bmpbuttn.h"
#include "wx/sizer.h"
#include "wx/tglbtn.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/geo-func.hh"
#include "geo/line.hh"
#include "gui/events.hh"
#include "gui/tool-drop-down-button.hh"
#include "text/formatting.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"

namespace faint{

ToolInfo::ToolInfo(const wxBitmap& inactive,
  const wxBitmap& active,
  const Tooltip& tooltip,
  const Description& description,
  const ToolId& toolId)
  : inactive(inactive),
    active(active),
    tooltip(tooltip),
    description(description),
    toolId(toolId)
{}

static void draw_drop_down_marker(Bitmap& b){
  int dx = 6;
  int dy = 3;
  int spacing = 2;
  IntPoint p1(b.m_w - dx - 1 - spacing, b.m_h - dy - 1 - spacing);
  IntPoint p2(p1 + delta_xy(dx / 2, dy));
  IntPoint p3(p1 + delta_x(dx));

  fill_triangle_color(b, p1, p2, p3, color_white);

  // Draw the triangle edge
  auto s = solid_1px(color_black);
  draw_line(b, {p1, p2}, s);
  draw_line(b, {p2, p3}, s);

  // The border looks a bit nicer if the top-line is shifted one step
  // up, <../doc/drop-down-shifted-top-line.png>.
  const auto shift = delta_y(-1);
  draw_line(b, {p1 + shift, p3 + shift}, s);
}

static wxBitmap drop_down_bitmap(const wxBitmap& src, const IntSize& outerSize){
  // Adds a drop-down indicating triangle to the bitmap
  IntSize innerSize(to_faint(src.GetSize()));
  IntPoint offset(point_from_size(outerSize - innerSize)/ 2);
  Bitmap b(outerSize, color_transparent_black);

  blit_masked(offsat(to_faint(src), offset), onto(b),
    Color(1,0,0));
  draw_drop_down_marker(b);
  return to_wx_bmp(b);
}

static void set_bitmaps(wxBitmapToggleButton* b,
  const IntSize& buttonSize,
  const ToolInfo& item)
{
  auto active(drop_down_bitmap(item.active, buttonSize));
  auto inactive(drop_down_bitmap(item.inactive, buttonSize));
  b->SetBitmap(inactive);
  b->SetBitmapFocus(active);
  b->SetBitmapCurrent(active);
  b->SetBitmapPressed(active);
}

ToolDropDownButton::ToolDropDownButton(wxWindow* parent,
  const std::vector<ToolInfo>& items,
  const IntSize& size)
  : wxBitmapToggleButton(parent, wxID_ANY,
      drop_down_bitmap(items.front().inactive, size),
      wxDefaultPosition,
      to_wx(size),
      wxWANTS_CHARS|wxBORDER_NONE),
    m_items(items),
    m_buttonSize(size)
{
  set_bitmaps(this, m_buttonSize, items.front());
}

bool ToolDropDownButton::SetSelectedTool(ToolId toolId){
  for (auto item : m_items){
    if (item.toolId == toolId){
      set_bitmaps(this, m_buttonSize, item);
      SetValue(true);
      return true;
    }
  }
  SetValue(false);
  return false;
}

void ToolDropDownButton::RefreshHover(){
  // This is workaround for sticking hover-state when the popup was
  // opened, but no tool in it selected (i.e. clicked outside). It
  // seems the button gets no wxEVT_LEAVE, so the hover appearance is
  // never cleared.
  if (GetValue()){
    SetValue(false);
    SetValue(true);
  }
  else{
    SetValue(false);
  }
}

class ToolPopup : public wxPopupWindow{
  // The popup-window shown when clicking the ToolDropDownButton
public:
  ToolPopup(ToolDropDownButton* parent,
    const std::vector<ToolInfo>& items,
    const IntSize& buttonSize)
    : wxPopupWindow(parent, wxBORDER_SIMPLE)
  {
    auto boxSizer = make_wx<wxBoxSizer>(wxVERTICAL);
    for (const auto& item : items){
      auto b = new wxBitmapToggleButton(this, wxID_ANY,
        item.inactive,
        wxDefaultPosition,
        to_wx(buttonSize),
        wxWANTS_CHARS);
      m_buttons.push_back({b, item});
      boxSizer->Add(b);
    }

    auto outerSizer = make_wx<wxBoxSizer>(wxHORIZONTAL);
    outerSizer->Add(boxSizer,
      0, wxALL, 5);
    SetSizerAndFit(outerSizer);

    events::on_mouse_left_down(this,
      [parent, this, buttonSize](const IntPoint& p){
        wxSize sz(GetSize());
        if (p.x < 0 || p.y < 0 || p.x >= sz.GetWidth() || p.y >= sz.GetHeight()){
          // Missed all buttons
          ReleaseMouse();
          Hide();
          parent->RefreshHover();
          return;
        }

        for (const auto& [button, toolInfo] : m_buttons){
          if (button->GetRect().Contains(to_wx(p))){
            // Hit a button
            ReleaseMouse();
            Hide();
            events::tool_change(parent, toolInfo.toolId);
          }
        }
      });

    bind(this, wxEVT_RIGHT_DOWN,
      [this](){
        ReleaseMouse();
        Hide();
      });

    bind(this, wxEVT_MOUSE_CAPTURE_LOST,
      [&](){
        Hide();
      });
  }
private:
  std::vector<std::pair<wxBitmapToggleButton*, ToolInfo>> m_buttons;
};

ToolDropDownButton* tool_drop_down_button(wxWindow* parent, const IntSize& size,
    const std::vector<ToolInfo>& items)
{
  auto b = make_wx<ToolDropDownButton>(parent, items, size);
  auto popup = make_wx<ToolPopup>(b, items, size);
  bind(b, wxEVT_TOGGLEBUTTON,
    [=](){
      popup->Show();
      popup->CaptureMouse();
      popup->Position(b->GetScreenPosition() - popup->GetSize(),
        popup->GetSize());
    });
  return b;
}

} // namespace
