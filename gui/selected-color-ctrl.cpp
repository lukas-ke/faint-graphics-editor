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

#include "wx/dcclient.h"
#include "wx/menu.h"
#include "wx/panel.h"
#include "bitmap/bitmap.hh"
#include "bitmap/paint.hh"
#include "geo/geo-func.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/size.hh"
#include "gui/color-data-object.hh"
#include "gui/dialog-context.hh"
#include "gui/events.hh"
#include "gui/paint-dialog.hh"
#include "gui/selected-color-ctrl.hh"
#include "gui/setting-events.hh"
#include "text/formatting.hh"
#include "util/color-bitmap-util.hh"
#include "util/color-choice.hh"
#include "util/optional.hh"
#include "util/setting-id.hh"
#include "util/status-interface.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"

namespace faint{

// Context-menu items
static const int menu_swap = wxNewId();
static const int menu_add = wxNewId();
static const int menu_copyRgb = wxNewId();
static const int menu_copyHex = wxNewId();

static LineStyle line_style(const FlaggedPaint& p){
  return p.second? LineStyle::LONG_DASH : LineStyle::SOLID;
}

static Color border_color(const FlaggedPaint& p){
  // - With solid border, black is OK regardless of the surrounded
  // color, as the solid border is just a decoration.
  //
  // - With a dashed border, use a brighter border around darker colors to
  // ensure it is easily visible, as the dashes indicate that the color
  // targets a selection.
  return line_style(p) == LineStyle::SOLID ?
    color_black :
    discernible_border_color(p.first);
}

static wxBitmap selected_color_bitmap(const FlaggedPaint& p,
  const IntSize& size)
{
  return to_wx_bmp(with_border(paint_bitmap(p.first, size),
    border_color(p), line_style(p)));
}

class SelectedColorCtrlImpl : public wxPanel, public ColorDropTarget {
public:
  using Which = SelectedColorCtrl::Which;
  SelectedColorCtrlImpl(wxWindow* parent,
    const IntSize& size,
    StatusInterface& statusInfo,
    DialogContext& dialogContext)
    : wxPanel(parent, wxID_ANY),
      ColorDropTarget(this),
      m_dialogContext(dialogContext),
      m_fg(Color(0,0,0)),
      m_bg(Color(0,0,0)),
      m_menuEventColor(Which::HIT_NEITHER),
      m_statusInfo(statusInfo)
  {
    const IntPoint p0 = IntPoint::Both(0);
    const IntSize rectSize = floored(floated(size) / 1.5);
    m_fgRect = IntRect(p0, rectSize);
    m_bgRect = IntRect(p0 + point_from_size(rectSize / 2), rectSize);

    m_fgBmp = to_wx_bmp(paint_bitmap(m_fg, m_fgRect.GetSize()));
    m_bgBmp = to_wx_bmp(paint_bitmap(m_bg, m_bgRect.GetSize()));

    #ifdef __WXMSW__
    // Prevent flicker on full refresh
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    #endif
    SetInitialSize(to_wx(size));

    events::on_paint(this, [this](){
      wxPaintDC dc(this);
      #ifdef __WXMSW__
      // Clear the background (for wxBG_STYLE_PAINT). Not required on GTK,
      // and GetBackgroundColour gives a darker gray for some reason
      dc.SetBackground(GetBackgroundColour());
      dc.Clear();
      #endif
      dc.DrawBitmap(m_bgBmp, m_bgRect.x, m_bgRect.y);
      dc.DrawBitmap(m_fgBmp, m_fgRect.x, m_fgRect.y);
    });

    events::on_mouse_left_down(this,
      [this](const IntPoint& pos){
        Which hit = HitTest(pos);
        if (hit == Which::HIT_NEITHER){
          return;
        }
        m_statusInfo.SetMainText("");
        const Paint& paint(GetClickedPaint(hit));
        wxString title(hit == Which::HIT_FG ?
          "Select Foreground Color" :
          "Select Background Color");

        show_paint_dialog(nullptr,
          title,
          paint,
          m_statusInfo,
          m_dialogContext).Visit(
            [&](const Paint& picked){
              SendChangeEvent(ToSetting(hit), picked);
            });
      });

    events::on_mouse_right_down(this,
      [this](const IntPoint& pos){
        // Show the context menu for the right clicked color
        m_statusInfo.SetMainText("");
        m_menuEventColor = HitTest(pos);
        wxMenu contextMenu;
        contextMenu.Append(menu_swap, "Swap colors");
        contextMenu.Append(menu_add, "Add to palette");
        contextMenu.AppendSeparator();
        contextMenu.Append(menu_copyHex, "Copy hex");
        contextMenu.Append(menu_copyRgb, "Copy rgb");

        // Disable items that require a specific color if neither the
        // foreground or background rectangles were hit
        if (m_menuEventColor == Which::HIT_NEITHER){
          contextMenu.Enable(menu_add, false);
          contextMenu.Enable(menu_copyHex, false);
          contextMenu.Enable(menu_copyRgb, false);
        }
        else{
          bool color = GetClickedPaint(m_menuEventColor).IsColor();
          if (!color){
            // Only colors can be copied as rgb or hex
            contextMenu.Enable(menu_copyHex, false);
            contextMenu.Enable(menu_copyRgb, false);
          }
        }
        PopupMenu(&contextMenu, to_wx(pos));
      });

    events::on_mouse_motion(this,
      [this](const IntPoint& pos){
        Which hit = HitTest(pos);

        if (hit == Which::HIT_NEITHER){
          m_statusInfo.SetMainText("");
          m_statusInfo.SetText("", 0);
        }
        else {
          m_statusInfo.SetMainText(
            "Left click for color dialog, right click for options.");
          m_statusInfo.SetText(str(GetClickedPaint(hit)), 0);
        }
      });

    events::on_mouse_leave_window(this, [this](){
      m_statusInfo.SetMainText("");
    });

    events::on_menu_choice_all(this,
      [this](int eventId){
        HandleMenuChoice(eventId);
      });
  }

  void UpdateColors(const ColorChoice& colors){
    m_fgBmp = selected_color_bitmap(colors.fg, m_fgRect.GetSize());
    m_bgBmp = selected_color_bitmap(colors.bg, m_bgRect.GetSize());
    m_fg = colors.fg.first;
    m_bg = colors.bg.first;
    Refresh();
  }

private:
  wxBitmap GetBitmap(const Color&);
  const Paint& MenuTargetColor() const{
    // Return the color the current right click menu refers to
    return GetClickedPaint(m_menuEventColor);
  }

  const Paint& GetClickedPaint(Which hit) const{
    if (hit == Which::HIT_FG){
      return m_fg;
    }
    else if (hit == Which::HIT_BG){
      return m_bg;
    }
    assert(false);
    return m_fg;
  }

  Which HitTest(const IntPoint& pos){
    if (m_fgRect.Contains(pos)){
      return Which::HIT_FG;
    }
    else if (m_bgRect.Contains(pos)){
      return Which::HIT_BG;
    }
    else {
      return Which::HIT_NEITHER;
    }
  }

  wxDragResult OnDropColor(const IntPoint& pos, const Color& color) override{
    // Check which item the color was dropped on and report this
    // upwards.
    Which hit = HitTest(pos);
    if (hit == Which::HIT_FG || hit == Which::HIT_NEITHER){
      SendChangeEvent(ts_Fg, Paint(color));
    }
    else if (hit == Which::HIT_BG){
      SendChangeEvent(ts_Bg, Paint(color));
    }
    return wxDragCopy;
  }

  void HandleMenuChoice(int action){
    if (action == menu_swap){
      wxCommandEvent newEvent(EVT_SWAP_COLORS);
      ProcessEvent(newEvent);
      return;
    }
    else{
      Paint paint(MenuTargetColor());
      if (action == menu_add){
        PaintEvent newEvent(FAINT_ADD_TO_PALETTE, paint);
        ProcessEvent(newEvent);
      }
      else if (action == menu_copyHex){
        assert(paint.IsColor()); // Should be unavailable for other than plain color
        ColorEvent newEvent(FAINT_COPY_COLOR_HEX, paint.GetColor());
        ProcessEvent(newEvent);
      }
      else if (action == menu_copyRgb){
        assert(paint.IsColor()); // Should be unavailable for other than plain color
        ColorEvent newEvent(FAINT_COPY_COLOR_RGB, paint.GetColor());
        ProcessEvent(newEvent);
      }
    }
  }

  void SendChangeEvent(const ColorSetting& setting, const Paint& value){
    SettingEvent<ColorSetting> event(setting, value,
      FAINT_COLOR_SETTING_CHANGE);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
  }

  static ColorSetting ToSetting(Which hit){
    assert(hit != Which::HIT_NEITHER);
    return hit == Which::HIT_FG ?
      ts_Fg : ts_Bg;
  }

  DialogContext& m_dialogContext;
  IntRect m_fgRect;
  IntRect m_bgRect;
  Paint m_fg;
  Paint m_bg;
  wxBitmap m_fgBmp;
  wxBitmap m_bgBmp;
  Which m_menuEventColor;
  StatusInterface& m_statusInfo;
};

SelectedColorCtrl::SelectedColorCtrl(wxWindow* parent,
  const IntSize& size,
  StatusInterface& status,
  DialogContext& context)

{
  m_impl = new SelectedColorCtrlImpl(parent, size, status, context);
}

SelectedColorCtrl::~SelectedColorCtrl(){
  m_impl = nullptr; // Deletion is handled by wxWidgets.
}

wxWindow* SelectedColorCtrl::AsWindow(){
  return m_impl;
}

void SelectedColorCtrl::UpdateColors(const ColorChoice& colors){
  m_impl->UpdateColors(colors);
}

} // namespace
