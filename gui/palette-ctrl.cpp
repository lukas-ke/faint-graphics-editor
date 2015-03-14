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

#include "wx/dcclient.h" // wxPaintDC
#include "wx/panel.h"
#include "bitmap/bitmap.hh"
#include "geo/measure.hh"
#include "gui/color-data-object.hh"
#include "gui/drop-source.hh"
#include "gui/events.hh"
#include "gui/mouse-capture.hh"
#include "gui/paint-dialog.hh"
#include "gui/palette-ctrl.hh"
#include "gui/setting-events.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util/color-bitmap-util.hh"
#include "util/optional.hh"
#include "util/paint-map.hh"
#include "util/setting-id.hh"
#include "util/settings.hh"
#include "util/status-interface.hh"

namespace faint{

static Optional<CellPos> highlight(const CellPos& pos){
  return Optional<CellPos>(pos);
}

static Optional<CellPos> no_highlight(){
  return no_option();
}

class PaletteCtrl::PaletteCtrlImpl : public wxPanel, public ColorDropTarget {
public:
  PaletteCtrlImpl(wxWindow* parent,
    const Settings& s,
    const PaintMap& palette,
    StatusInterface& status,
    DialogContext& dialogContext)
    : wxPanel(parent, wxID_ANY),
      ColorDropTarget(this),
      m_dialogContext(dialogContext),
      m_mouse(this),
      m_settings(s),
      m_statusInterface(status)
  {
    #ifdef __WXMSW__
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Prevent flicker on full refresh
    #endif

    SetPalette(palette);

    events::on_mouse_leave_window(this, [&](){
      m_statusInterface.SetMainText("");
    });

    events::on_mouse_left_double_click(this,
      [&](const IntPoint& pos){
        CellPos cellPos = MousePosToPalettePos(pos);
        if (m_paintMap.Has(cellPos)){
          ShowColorDialog(cellPos);
        }
      });

    events::on_mouse_left_down(this,
      [&](const IntPoint& pos){
        m_dragStart = pos;
        CellPos cellPos = MousePosToPalettePos(m_dragStart);
        if (!m_paintMap.Has(cellPos)){
          return;
        }

        Paint fg(m_paintMap.Get(cellPos));
        SetFg(fg);

        // Capture the mouse to determine distance for drag and drop
        m_mouse.Capture();
      });

    events::on_mouse_left_up(this, releaser(m_mouse));

    events::on_mouse_motion(this,
      [&](const IntPoint& pos){
        UpdateStatusInterface(pos);
        if (m_mouse.HasCapture()){
          UpdateCapture(pos);
        }
      });

    events::on_paint(this, [&](){
      wxPaintDC dc(this);
      #ifdef __WXMSW__
      // Clear the background (for wxBG_STYLE_PAINT). Not required on GTK,
      // where GetBackgroundColour also gives a darker gray for some
      // reason
      dc.SetBackground(GetBackgroundColour());
      dc.Clear();
      #endif
      dc.DrawBitmap(m_bitmap, 0, 0);
    });

    auto handle_right_click =
      [&](const IntPoint& pos){
        // Lambda for binding both single and double right-click.
        CellPos cellPos = MousePosToPalettePos(pos);
        if (!m_paintMap.Has(cellPos)){
          return;
        }
        bool ctrlHeld = wxGetKeyState(WXK_CONTROL);
        if (ctrlHeld){
          RemoveAt(cellPos);
        }
        else {
          SetBg(m_paintMap.Get(cellPos));
        }
      };

    events::on_mouse_right_down(this, handle_right_click);
    events::on_mouse_right_double_click(this, handle_right_click);
  }

  void Add(const Paint& paint){
    m_paintMap.Append(paint);
    CreateBitmap(no_highlight());
  }

  void SetPalette(const PaintMap& paintMap){
    m_paintMap = paintMap;
    CreateBitmap(no_highlight());
  }

private:
  void CreateBitmap(const Optional<CellPos>& maybeHighlight){
    CellSize cellSize(GetButtonSize());
    CellSpacing spacing(GetButtonSpacing());
    Color bg(to_faint(GetBackgroundColour()));
    Bitmap bmp(m_paintMap.CreateBitmap(cellSize, spacing, bg));

    maybeHighlight.Visit(
      [&](const CellPos& highlight){
        add_cell_border(bmp, highlight, cellSize, spacing,
          discernible_border_color(m_paintMap.Get(highlight)));
      });

    m_bitmap = to_wx_bmp(bmp);
    SetSize(m_bitmap.GetSize());
    SetInitialSize(m_bitmap.GetSize());
    send_control_resized_event(this);
    Refresh();
    return;
  }

  CellSize GetButtonSize() const{
    return CellSize(IntSize(24, 24));
  }

  CellSpacing GetButtonSpacing() const{
    return CellSpacing(IntSize(2,2));
  }

  CellPos MousePosToPalettePos(const IntPoint& pos) const{
    return view_to_cell_pos(pos, GetButtonSize(), GetButtonSpacing());
  }

  wxDragResult OnDropColor(const IntPoint& pos, const Color& /*color*/) override{
    // Fixme: This ignores the dropped color and only uses indexes, while assumes this panel is the source. This makes the drag and drop work for patterns etc. despite no support in ColorDataObject, but isn't proper dnd. For example, this will misbehave when dragging from another Faint instance.

    bool ctrlHeld = wxGetKeyState(WXK_CONTROL);
    if (ctrlHeld){
      m_paintMap.Copy(Old(MousePosToPalettePos(m_dragStart)),
        New(MousePosToPalettePos(pos)));
    }
    else {
      m_paintMap.Move(Old(MousePosToPalettePos(m_dragStart)),
        New(MousePosToPalettePos(pos)));
    }
    CreateBitmap(no_highlight());
    return wxDragMove;
  }

  void RemoveAt(const CellPos& pos){
    m_paintMap.Erase(pos);
    CreateBitmap(no_highlight());
  }

  void SendChangeEvent(const ColorSetting& setting, const Paint& value){
    SettingEvent<ColorSetting> event(setting, value,
      FAINT_COLOR_SETTING_CHANGE);
    event.SetEventObject(this);
    GetEventHandler()->ProcessEvent(event);
  }

  void SetBg(const Paint& bg){
    SendChangeEvent(ts_Bg, bg);
  }

  void SetFg(const Paint& fg){
    m_prevFg = m_settings.Get(ts_Fg);
    SendChangeEvent(ts_Fg, fg);
  }

  void ShowColorDialog(const CellPos& pos){
    // Highlight the edited color in the palette
    CreateBitmap(highlight(pos));

    show_paint_dialog(nullptr,
      "Edit Palette Color",
      m_paintMap.Get(pos),
      m_statusInterface,
      m_dialogContext).Visit(
        [&](const Paint& paint){
          m_paintMap.Replace(pos, paint);
          SetFg(paint);
        });

    // Clear the highlight
    CreateBitmap(no_highlight());
  }

  void UndoSetFg(){
    SendChangeEvent(ts_Fg, m_prevFg);
  }

  void UpdateCapture(const IntPoint& pos){
    assert(m_mouse.HasCapture());

    const int minDistance(GetButtonSize().w / 2);
    const int dragDistance = floored(distance(pos, m_dragStart));
    if (dragDistance <= minDistance){
      // Haven't dragged far enough yet to discern if dragging or just clicking
      return;
    }

    // Start dragging the color
    m_mouse.Release();

    // The selected foreground color should not be changed when the
    // click initiates dragging of a color.
    UndoSetFg();

    CellPos cellPos(MousePosToPalettePos(m_dragStart));
    Paint paint(m_paintMap.Get(cellPos));
    Color c(paint.IsColor() ? paint.GetColor() : Color(0,0,0)); // Fixme: ColorDataObject doesn't support pattern, gradient
    ColorDataObject colorObj(c);
    FaintDropSource source(this, colorObj);
    source.CustomDoDragDrop();
  }

  void UpdateStatusInterface(const IntPoint& pos){
    auto cellPos = MousePosToPalettePos(pos);
    if (m_paintMap.Has(cellPos)){
      const Paint& paint = m_paintMap.Get(cellPos);
      m_statusInterface.SetMainText(str(paint));
    }
    else {
      m_statusInterface.SetMainText("");
    }
  }

  DialogContext& m_dialogContext;
  MouseCapture m_mouse;
  const Settings& m_settings;
  Paint m_prevFg;
  wxBitmap m_bitmap;
  IntPoint m_dragStart;
  Optional<CellPos> m_highLight;
  PaintMap m_paintMap;
  StatusInterface& m_statusInterface;
};

PaletteCtrl::PaletteCtrl(wxWindow* parent,
  const Settings& settings,
  const PaintMap& palette,
  StatusInterface& status,
  DialogContext& context)
  : m_impl(make_dumb<PaletteCtrlImpl>(parent, settings, palette, status, context))
{}

wxWindow* PaletteCtrl::AsWindow(){
  return m_impl.get();
}

void PaletteCtrl::Add(const Paint& paint){
  m_impl->Add(paint);
}

void PaletteCtrl::SetPalette(const PaintMap& paintMap){
  m_impl->SetPalette(paintMap);
}

} // namespace
