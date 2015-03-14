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
#include "wx/event.h"
#include "wx/panel.h"
#include "wx/textctrl.h"
#include "app/canvas.hh"
#include "app/get-app-context.hh" // For use image
#include "app/get-art-container.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "bitmap/pattern.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/line.hh"
#include "gui/art-container.hh"
#include "gui/layout.hh"
#include "gui/mouse-capture.hh"
#include "gui/paint-dialog/pattern-panel.hh"
#include "text/formatting.hh"
#include "util/color-bitmap-util.hh"
#include "util/color-span.hh"
#include "util/either.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/clipboard.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/make-event.hh"
#include "util-wx/placement.hh"
#include "util/named-operator.hh"

namespace faint{

MAKE_FAINT_COMMAND_EVENT(CLICKED_ANCHOR);

static void draw_cross(Bitmap& bmp,
  const IntPoint& center,
  int r,
  int lineWidth,
  const Color& c)
{
  const LineSettings s(c, lineWidth, LineStyle::SOLID, LineCap::BUTT);

  draw_hline(bmp, center.y, center.x <plus_minus> r, s);
  draw_vline(bmp, center.x, center.y <plus_minus> r, s);
}

class PatternDisplay : public wxPanel{
public:
  PatternDisplay(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_hovered(false),
      m_mouse(this),
      m_pattern(default_pattern(IntSize(240,240)))
  {
    SetInitialSize(to_wx(m_pattern.GetSize()));
    SetBackgroundStyle(wxBG_STYLE_PAINT); // Avoid flicker on msw

    events::on_mouse_enter_window(this, [&](){
      m_hovered = true;
      Refresh();
    });

    events::on_mouse_leave_window(this, [this](){
      m_hovered = false;
      Refresh();
    });

    events::on_mouse_left_down(this, [this](const IntPoint& pos){
        m_mouse.Capture();
        SetAnchorFromPos(pos);
      });

    events::on_mouse_left_up(this, releaser(m_mouse));

    events::on_mouse_motion(this,
      [this](const IntPoint& pos){
        m_hovered = true;
        if (m_mouse.HasCapture()){
          SetAnchorFromPos(pos);
          SetCursor(get_art_container().Get(Cursor::BLANK));
        }
        else{
          SetCursor(get_art_container().Get(Cursor::CROSSHAIR));
        }
      });

    events::on_paint(this, [this](){
      IntSize sz(to_faint(GetSize()));
      Bitmap buffer(sz);
      clear(buffer, Color(255,255,255));
      IntPoint center(point_from_size(sz / 2));
      Paint paint(offsat(Paint(m_pattern), -center));
      fill_rect(buffer, rect_from_size(sz), paint);
      if (!m_hovered){
        draw_cross(buffer, center, 11, 3, color_white);
      }
      draw_cross(buffer, center, 10, 1, color_black);
      wxPaintDC dc(this);
      dc.DrawBitmap(to_wx_bmp(buffer), 0, 0);
    });
  }

  IntPoint GetAnchor() const{
    return m_pattern.GetAnchor();
  }

  bool GetObjectAligned() const {
    return m_pattern.GetObjectAligned();
  }

  const Pattern& GetPattern() const {
    return m_pattern;
  }

  void SetAnchor(const IntPoint& anchor){
    m_pattern.SetAnchor(anchor);
  }

  void SetObjectAligned(bool objectAligned){
    m_pattern.SetObjectAligned(objectAligned);
  }

  void SetPattern(const Pattern& pattern){
    m_pattern = pattern;
  }
private:
  void SetAnchorFromPos(const IntPoint& pos){
    IntSize sz(to_faint(GetSize()));
    IntPoint center(point_from_size(sz / 2));
    SetAnchor(GetAnchor() + pos - center); // Fixme: Notify text controls
    WarpPointer(center.x, center.y);
    wxCommandEvent newEvent(EVT_CLICKED_ANCHOR);
    ProcessEvent(newEvent);
    Refresh();
  }

  bool m_hovered;
  MouseCapture m_mouse;
  Pattern m_pattern;
};

class PaintPanel_Pattern::PaintPanel_Pattern_Impl : public wxPanel{
public:
  PaintPanel_Pattern_Impl(wxWindow* parent)
    : wxPanel(parent, wxID_ANY),
      m_anchor(0,0),
      m_objectAligned(nullptr),
      m_patternDisplay(nullptr)
  {
    m_patternDisplay = new PatternDisplay(this);
    set_pos(m_patternDisplay, IntPoint::Both(panel_padding));

    auto btnUseImage = create_button(this, "Use Image", below(m_patternDisplay),
      [this](){
        // Fixme: Reveals everything.
        AppContext& app = get_app_context();
        const auto& background = app.GetActiveCanvas().GetBackground();
        background.Visit(
          [&](const Bitmap& bmp){
            m_patternDisplay->SetPattern(Pattern(bmp));
          },
          [&](const ColorSpan& colorSpan){
            // Not using the colorSpan size, since it might be huge, and
            // it would be pointless with only one color.
            const IntSize size(1,1);
            m_patternDisplay->SetPattern(Pattern(Bitmap(size, colorSpan.color)));
          });
        UpdateDisplay();
      });


    auto btnPaste = create_button(this, "Paste Pattern",
      to_the_right_of(raw(btnUseImage)),
      [this](){
        PastePattern();
      });

    create_button(this, "Copy Pattern",
      to_the_right_of(raw(btnPaste)),
      [this](){
        CopyPattern();
      });

    m_objectAligned = create_checkbox(this,
      "&Object Aligned",
      m_patternDisplay->GetObjectAligned(),
      to_the_right_of(m_patternDisplay),
      [&](){
        m_patternDisplay->SetObjectAligned(get(m_objectAligned));
      });

    m_anchorX = create_text_control(this, "0", below(raw(m_objectAligned)),
      Width(50));
    m_anchorY = create_text_control(this, "0", to_the_right_of(m_anchorX),
      Width(50));

    bind_fwd(this, wxEVT_COMMAND_TEXT_UPDATED,
      [this](wxCommandEvent& e){
        e.Skip();
        wxTextCtrl* textCtrl(dynamic_cast<wxTextCtrl*>(e.GetEventObject()));
        wxString text = textCtrl->GetValue();
        IntPoint oldAnchor(m_patternDisplay->GetAnchor());
        if (text.empty() || !text.IsNumber()){
          textCtrl->SetValue(to_wx(str_int(textCtrl == m_anchorX ?
                oldAnchor.x :
                oldAnchor.y)));
          return;
        }

        if (textCtrl == m_anchorX){
          m_patternDisplay->SetAnchor(IntPoint(parse_int_value(textCtrl,
            oldAnchor.x), oldAnchor.y));
        }
        else if (textCtrl == m_anchorY){
          m_patternDisplay->SetAnchor(IntPoint(oldAnchor.x,
              parse_int_value(textCtrl, oldAnchor.y)));
        }
        else{
          assert(false);
        }
        UpdateDisplay();
    });

    bind(this, EVT_CLICKED_ANCHOR,
      [this](){
        IntPoint anchor(m_patternDisplay->GetAnchor());
        m_anchorX->SetValue(to_wx(str_int(anchor.x)));
        m_anchorY->SetValue(to_wx(str_int(anchor.y)));
      });
  }

  void CopyPattern(){
    Clipboard clip;
    if (clip.Good()){
      // Fixme: Add pattern-function for clip-board, to retain anchor,
      // object aligned and so on
      clip.SetBitmap(m_patternDisplay->GetPattern().GetBitmap(), rgb_white);
    }
  }

  void SetPattern(const Pattern& pattern){
    m_patternDisplay->SetPattern(pattern);
    set(m_objectAligned, pattern.GetObjectAligned());

    IntPoint anchor(pattern.GetAnchor());
    m_anchorX->SetValue(to_wx(str_int(anchor.x)));
    m_anchorY->SetValue(to_wx(str_int(anchor.y)));
    UpdateDisplay();
  }

  const Pattern& GetPattern() const{
    return m_patternDisplay->GetPattern();
  }

  void PastePattern(){
    Clipboard clip;
    if (clip.Good()){
      if (auto maybeBmp = clip.GetBitmap()){
        const Bitmap bmp(maybeBmp.Take());
        if (bitmap_ok(bmp)){
          IntPoint anchor(m_patternDisplay->GetAnchor());
          m_anchorX->SetValue(wxString::Format("%d", anchor.x));
          m_anchorY->SetValue(wxString::Format("%d", anchor.y));
          m_patternDisplay->SetPattern(Pattern(std::move(bmp)));
        }
      }
    }
    UpdateDisplay();
  }

private:
  void UpdateDisplay(){
    m_patternDisplay->Refresh();
  }

  IntPoint m_anchor;
  wxCheckBox* m_objectAligned; // Fixme: Perhaps this is rather anchor than align
  PatternDisplay* m_patternDisplay;
  wxTextCtrl* m_anchorX;
  wxTextCtrl* m_anchorY;
};

PaintPanel_Pattern::PaintPanel_Pattern(wxWindow* parent){
  m_impl = new PaintPanel_Pattern_Impl(parent);
}

PaintPanel_Pattern::~PaintPanel_Pattern(){
  m_impl = nullptr; // Deletion handled by wxWidgets
}

wxWindow* PaintPanel_Pattern::AsWindow(){
  return m_impl;
}

void PaintPanel_Pattern::Copy(){
  m_impl->CopyPattern();
}

Pattern PaintPanel_Pattern::GetPattern() const{
  return m_impl->GetPattern();
}

void PaintPanel_Pattern::SetPattern(const Pattern& pattern){
  m_impl->SetPattern(pattern);
}

void PaintPanel_Pattern::Paste(){
  m_impl->PastePattern();
}

} // namespace
