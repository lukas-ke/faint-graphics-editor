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

#include <algorithm>
#include <sstream>
#include "wx/button.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/panel.h"
#include "wx/settings.h"
#include "wx/sizer.h"
#include "app/resource-id.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/measure.hh"
#include "gui/art.hh"
#include "gui/frame-ctrl.hh"
#include "gui/mouse-capture.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util/status-interface.hh"

namespace faint{

const int iconSpacing = 5;

#ifdef __WXMSW__
#define FRAMECTRL_BORDER_STYLE wxBORDER_THEME
#else
#define FRAMECTRL_BORDER_STYLE wxBORDER_NONE
#endif

struct FrameDragInfo{
  FrameDragInfo()
    : active(false),
      copy(false),
      dropPost(0),
      sourceFrame(0)
  {}

  FrameDragInfo(const IntPoint& dragStart, const Index& sourceFrame)
    : active(false),
      copy(false),
      dragStart(dragStart),
      dropPost(0),
      sourceFrame(sourceFrame)
  {}

  Index DropFrame() const{
    if (copy){
      // When copying, the index for the new frame is the same as the
      // index of the post.
      return dropPost;
    }

    // When a frame is moved, a drop on either post surrounding the
    // source frame maps to the original frame index. Therefore, when
    // dragging rightwards, the post-count must be lowered by one to
    // get the resulting frame index.
    return dropPost - (sourceFrame < dropPost ? 1 : 0);
  }

  bool WouldChange() const{
    return active && (copy || DropFrame() != sourceFrame);
  }

  bool active;
  bool copy;
  IntPoint dragStart;
  Index dropPost;
  Index sourceFrame;
};

static FrameDragInfo then_reset(FrameDragInfo& info){
  // Resets the drag info by default construction, returns the state
  // as a copy.
  FrameDragInfo oldState;
  std::swap(oldState, info);
  return oldState;
}

static void run_frame_drag_command(FrameContext& ctx,
  const FrameDragInfo& dragInfo)
{
  Index srcIndex(dragInfo.sourceFrame);
  Index dstIndex(dragInfo.DropFrame());
  if (dragInfo.copy){
    ctx.CopyFrame(Old(srcIndex), New(dstIndex));
  }
  else {
    ctx.MoveFrame(Old(srcIndex), New(dstIndex));
  }
  ctx.SelectFrame(dstIndex);
}

static utf8_string get_frame_label(const Index& index, bool capitalize){
  std::stringstream ss;
  ss << (capitalize ? "Frame " : "frame ") << index.Get() + 1;
  return utf8_string(ss.str());
}

class FrameListCtrl : public wxPanel {
public:
  FrameListCtrl(wxWindow* parent,
    FrameContext& ctx,
    const Art& art,
    StatusInterface& status)
    : wxPanel(parent),
      m_closeFrameBitmap(art.Get(Icon::CLOSE_FRAME)),
      m_closeFrameHighlightBitmap(art.Get(Icon::CLOSE_FRAME_HIGHLIGHT)),
      m_ctx(ctx),
      m_cursorArrow(art.Get(Cursor::ARROW)),
      m_cursorDragCopyFrame(art.Get(Cursor::DRAG_COPY_FRAME)),
      m_cursorDragFrame(art.Get(Cursor::DRAG_FRAME)),
      m_frameBoxSize(35, 38),
      m_highlightCloseFrame(false),
      m_mouse(this,
        OnLoss([this](){
          // Discard any drag and then refresh this control to get rid of drag
          // overlay graphics
          if (then_reset(m_dragInfo).active){
            Refresh();
          }
        }),
        OnRelease([this](){
          SetCursor(wxCURSOR_ARROW);
        })),
      m_numFrames(0),
      m_status(status)
  {
    #ifdef __WXMSW__
    SetBackgroundStyle(wxBG_STYLE_PAINT);
    #endif

    events::on_mouse_leave_window(this, [this](){
      m_status.SetMainText("");
      SetHighlightCloseFrame(false);
    });

    auto handle_click =
      [&](const IntPoint& pos){
        if (m_mouse.HasCapture()){
          return;
        }

        Index frame = GetFrameIndex(pos, false);
        Index selected = m_ctx.GetSelectedFrame();
        if (frame == selected && CloseBoxHitTest(pos, frame.Get())){
          m_ctx.RemoveFrame(frame);
        }

        assert(frame < m_ctx.GetNumFrames());
        m_ctx.SelectFrame(frame);
        m_dragInfo = FrameDragInfo(pos, frame);
        m_mouse.Capture();
        Refresh();
    };

    events::on_mouse_left_down(this, handle_click);
    events::on_mouse_left_double_click(this, handle_click);

    events::on_mouse_left_up(this,
      [this](const IntPoint&){
        m_mouse.Release();
        UpdateDragKeyState(wxGetKeyState(WXK_CONTROL));
        FrameDragInfo dragResult(then_reset(m_dragInfo));
        if (dragResult.WouldChange()){

          run_frame_drag_command(m_ctx, dragResult);
          Refresh();
        }
        else{
          // Refresh to clear the drop overlay
          Refresh();
        }
      });

    events::on_mouse_motion(this,
      [this](const IntPoint& pos){
        if (!m_mouse.HasCapture()){
          Index frame = GetFrameIndex(pos, true);
          if (frame == m_ctx.GetSelectedFrame() && CloseBoxHitTest(pos,
              frame.Get()))
          {
            m_status.SetMainText("Click to remove " +
              get_frame_label(frame, false));
            SetHighlightCloseFrame(true);
          }
          else {
            SetHighlightCloseFrame(false);
            m_status.SetMainText(get_frame_label(frame, true));
            SetCursor(m_cursorArrow);
          }
        }
        else{
          SetHighlightCloseFrame(false);
          bool ctrlHeld = wxGetKeyState(WXK_CONTROL);
          if (m_dragInfo.active){
            UpdateDragKeyState(ctrlHeld);

            Index frame = GetDropFrameIndex(pos);
            if (m_dragInfo.dropPost != frame){
              m_dragInfo.dropPost = frame;
              Refresh();
            }
          }
          else if (distance(pos, m_dragInfo.dragStart) > 5) {
            m_dragInfo.active = true;
            if (ctrlHeld){
              SetCursor(m_cursorDragCopyFrame);
            }
            SetCursor(m_cursorDragFrame);
          }
        }
      });

    events::on_paint(this, [this](){
      CreateBitmap();
      wxPaintDC dc(this);
      dc.DrawBitmap(m_bitmap, GetDrawOffset(), 0);
    });

    events::on_mouse_right_down(this,
      [this](const IntPoint& pos){
        bool ctrlHeld = wxGetKeyState(WXK_CONTROL);
        if (!ctrlHeld){
          return;
        }
        Index index = GetFrameIndex(pos, false);
        m_ctx.RemoveFrame(index);
      });
  }

  void CreateBitmap(){
    const int iconWidth = m_frameBoxSize.GetWidth();
    const int iconHeight = m_frameBoxSize.GetHeight();

    // The bmp should be at least as large as the panel
    int bmpWidth = std::max((iconWidth + iconSpacing) * m_numFrames.Get(),
      GetSize().GetWidth());

    m_bitmap = wxBitmap(wxSize(bmpWidth, iconHeight));
    wxMemoryDC dc(m_bitmap);
    dc.SetFont(wxFont(wxFontInfo(8).Family(wxFONTFAMILY_MODERN)));
    dc.SetBackground(wxSystemSettings::GetColour(wxSYS_COLOUR_BTNFACE));
    dc.Clear();
    int selected = m_ctx.GetSelectedFrame().Get();
    wxPen activePen(wxColour(0,0,0));
    wxPen inactivePen(wxColour(0,0,0));
    wxBrush activeBrush(wxColour(255,255,255));
    wxBrush inactiveBrush(wxColour(128,128,128));

    const int numFrames(m_numFrames.Get());
    for (int i = 0; i != numFrames; i++){
      if (i == selected){
        dc.SetPen(activePen);
        dc.SetBrush(activeBrush);
      }
      else {
        dc.SetPen(inactivePen);
        dc.SetBrush(inactiveBrush);
      }
      wxPoint pos(i * (iconWidth + iconSpacing) + iconSpacing, 0);
      dc.DrawRectangle(pos, m_frameBoxSize);
      if (i == selected){
        if (m_highlightCloseFrame){
          dc.DrawBitmap(m_closeFrameHighlightBitmap, pos +
            wxPoint(iconWidth - m_closeFrameHighlightBitmap.GetWidth() - 1, 1),
            true); // Use mask
        }
        else{
          dc.DrawBitmap(m_closeFrameBitmap, pos +
            wxPoint(iconWidth - m_closeFrameBitmap.GetWidth() - 1, 1),
            true); // Use mask
        }
      }
      dc.DrawLabel(to_wx(str_user(Index(i))), wxRect(pos, m_frameBoxSize),
        wxALIGN_RIGHT|wxALIGN_BOTTOM);
    }
    if (m_dragInfo.active){
      wxPen dragPen(wxColour(0,0,0), 3, wxPENSTYLE_SOLID);
      dragPen.SetCap(wxCAP_BUTT);
      dc.SetPen(dragPen);
      int dropPost = m_dragInfo.dropPost.Get();
      dc.DrawLine(dropPost * (iconWidth + iconSpacing) + iconSpacing - 3, 0,
        dropPost * (iconWidth + iconSpacing) + iconSpacing - 3, iconHeight);
    }
  }

  Index GetShownFrames() const{
    return m_numFrames;
  }

  void SetNumFrames(const Index& numFrames){
    SetInitialSize(wxSize((
      m_frameBoxSize.GetWidth() + iconSpacing) * numFrames.Get() + iconSpacing,
        m_frameBoxSize.GetHeight()));
    m_numFrames = numFrames;
    Refresh();
  }

private:
  void UpdateDragKeyState(bool ctrlHeld){
    if (ctrlHeld){
      if (!m_dragInfo.copy){
        m_dragInfo.copy = true;
        SetCursor(m_cursorDragCopyFrame);
      }
    }
    else if (m_dragInfo.copy){
      SetCursor(m_cursorDragFrame);
      m_dragInfo.copy = false;
    }
  }

  int GetDrawOffset(){
    wxSize sz = GetSize();
    if (m_bitmap.GetWidth() > sz.GetWidth()){
      int x = m_ctx.GetSelectedFrame().Get() *
        (m_frameBoxSize.GetWidth() + iconSpacing);
      if (x > sz.GetWidth() / 2){
        return std::max(-x - (m_frameBoxSize.GetWidth() + iconSpacing) +
          sz.GetWidth() / 2,
          -m_bitmap.GetWidth() + sz.GetWidth());
      }
    }
    return 0;
  }

  Index GetFrameIndex(const IntPoint& pos, bool allowNext){
    // Gets the frame under pos
    Index numFrames(m_ctx.GetNumFrames());
    Index frame(std::max((pos.x - GetDrawOffset()) /
      (m_frameBoxSize.GetWidth() + iconSpacing), 0));
    if (frame < numFrames){
      return frame;
    }
    else{
      return allowNext ? numFrames : numFrames - 1;
    }
  }

  Index GetDropFrameIndex(const IntPoint& pos){
    // During drag and drop, the drop should be after the hovered frame
    // if the hovering is past its mid-point.
    // <../doc/frame-mid-point.png>
    const IntPoint offset(m_frameBoxSize.GetWidth() / 2, 0);
    return GetFrameIndex(pos + offset, true);
  }

  bool CloseBoxHitTest(const IntPoint& pos, int index){
    const int x = index * (m_frameBoxSize.GetWidth() + iconSpacing) +
      iconSpacing +
      m_frameBoxSize.GetWidth() -
      m_closeFrameBitmap.GetWidth() - 1 +
      GetDrawOffset();
    const IntRect closeRect(IntPoint(x, 0), get_size(m_closeFrameBitmap));
    return closeRect.Contains(pos);
  }

  void SetHighlightCloseFrame(bool highlight){
    if (highlight != m_highlightCloseFrame){
      m_highlightCloseFrame = highlight;
      Refresh();
    }
  }

  wxBitmap m_bitmap;
  wxBitmap m_closeFrameBitmap;
  wxBitmap m_closeFrameHighlightBitmap;
  FrameContext& m_ctx;
  wxCursor m_cursorArrow;
  wxCursor m_cursorDragCopyFrame;
  wxCursor m_cursorDragFrame;
  FrameDragInfo m_dragInfo;
  wxSize m_frameBoxSize;
  bool m_highlightCloseFrame;
  MouseCapture m_mouse;
  Index m_numFrames;
  StatusInterface& m_status;
};

class FrameCtrl::FrameCtrlImpl : public wxPanel {
public:
  FrameCtrlImpl(wxWindow* parent,
    std::unique_ptr<FrameContext>&& ctx,
    StatusInterface& status,
    const Art& art)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL|FRAMECTRL_BORDER_STYLE),
      m_ctx(std::move(ctx)),
      m_listCtrl(nullptr)
  {
    auto sizer = make_wx<wxBoxSizer>(wxHORIZONTAL);

    sizer->Add(noiseless_button(this,
      art.Get(Icon::ADD_FRAME),
      Tooltip("Add Frame"),
      IntSize(60,50),
      [=](){m_ctx->AddFrame();}));

    m_listCtrl = make_wx<FrameListCtrl>(this,
      *m_ctx,
      art,
      status);
    sizer->Add(m_listCtrl);
    SetSizer(sizer);
    Layout();
  }

  Index GetShownFrames() const {
    return m_listCtrl->GetShownFrames();
  }

  void SetNumFrames(const Index& numFrames){
    assert(numFrames.Get() != 0);
    m_listCtrl->SetNumFrames(numFrames);
    if (numFrames.Get() == 1){
      m_listCtrl->Hide();
    }
    else{
      m_listCtrl->Show();
    }
  }

  bool UpdateFrames(){
    const Index numFrames = m_ctx->GetNumFrames();
    if (GetShownFrames() == numFrames){
      m_listCtrl->Refresh();
      return false;
    }
    SetNumFrames(numFrames);
    return true;
  }

private:
  std::unique_ptr<FrameContext> m_ctx;
  FrameListCtrl* m_listCtrl;
};

FrameCtrl::FrameCtrl(wxWindow* parent,
  std::unique_ptr<FrameContext>&& ctx,
  StatusInterface& status,
  const Art& art)
{
  m_impl = make_wx<FrameCtrlImpl>(parent, std::move(ctx), status, art);
}

FrameCtrl::~FrameCtrl(){
  deleted_by_wx(m_impl);
}

wxWindow* FrameCtrl::AsWindow(){
  return m_impl;
}

bool FrameCtrl::Update(){
  return m_impl->UpdateFrames();
}

} // namespace
