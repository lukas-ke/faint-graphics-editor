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
#include "app/get-art-container.hh" // Fixme: pass it instead
#include "wx/button.h"
#include "wx/dcclient.h"
#include "wx/dcmemory.h"
#include "wx/panel.h"
#include "wx/settings.h"
#include "wx/sizer.h"
#include "app/app-context.hh"
#include "app/canvas.hh"
#include "commands/frame-cmd.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/measure.hh"
#include "gui/art-container.hh"
#include "gui/frame-ctrl.hh"
#include "gui/mouse-capture.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"

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

static void run_frame_drag_command(Canvas& canvas,
  const FrameDragInfo& dragInfo)
{
  Index srcIndex(dragInfo.sourceFrame);
  Index dstIndex(dragInfo.DropFrame());
  if (dragInfo.copy){
    canvas.RunCommand(add_frame_command(canvas.GetFrame(srcIndex), dstIndex));
  }
  else {
    canvas.RunCommand(reorder_frame_command(New(dstIndex), Old(srcIndex)));
  }
  canvas.SelectFrame(dstIndex);
}

static utf8_string get_frame_label(const Index& index, bool capitalize){
  std::stringstream ss;
  ss << (capitalize ? "Frame " : "frame ") << index.Get() + 1;
  return utf8_string(ss.str());
}

class FrameListCtrl : public wxPanel {
public:
  FrameListCtrl(wxWindow* parent,
    const wxBitmap& closeFrameBitmap,
    const wxBitmap& closeFrameHighlightBitmap,
    AppContext& app,
    StatusInterface& status)
    : wxPanel(parent),
      m_app(app),
      m_closeFrameBitmap(closeFrameBitmap),
      m_closeFrameHighlightBitmap(closeFrameHighlightBitmap),
      m_frameBoxSize(35, 38),
      m_highlightCloseFrame(false),
      m_mouse(this,
        [this](){
          // Capture lost:
          // Discard any drag and then refresh this control to get rid of drag
          // overlay graphics
          if (then_reset(m_dragInfo).active){
            Refresh();
          }
        },
        [this](){
          SetCursor(wxCURSOR_ARROW);
        }),
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
        Canvas& active = m_app.GetActiveCanvas();
        Index frame = GetFrameIndex(pos, active, false);
        Index selected = active.GetSelectedFrame();
        if (frame == selected && CloseBoxHitTest(pos, frame.Get())){
          active.RunCommand(remove_frame_command(frame));
          return;
        }

        assert(frame < active.GetNumFrames());
        active.SelectFrame(frame);
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
          run_frame_drag_command(m_app.GetActiveCanvas(), dragResult);
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
          Canvas& active = m_app.GetActiveCanvas();
          Index frame = GetFrameIndex(pos, active, true);
          if (frame == active.GetSelectedFrame() && CloseBoxHitTest(pos,
              frame.Get()))
          {
            m_status.SetMainText("Click to remove " +
              get_frame_label(frame, false));
            SetHighlightCloseFrame(true);
          }
          else {
            SetHighlightCloseFrame(false);
            m_status.SetMainText(get_frame_label(frame, true));
            SetCursor(get_art_container().Get(Cursor::ARROW));
          }
        }
        else{
          SetHighlightCloseFrame(false);
          bool ctrlHeld = wxGetKeyState(WXK_CONTROL);
          if (m_dragInfo.active){
            UpdateDragKeyState(ctrlHeld);

            Index frame = GetDropFrameIndex(pos, m_app.GetActiveCanvas());
            if (m_dragInfo.dropPost != frame){
              m_dragInfo.dropPost = frame;
              Refresh();
            }
          }
          else if (distance(pos, m_dragInfo.dragStart) > 5) {
            m_dragInfo.active = true;
            if (ctrlHeld){
              SetCursor(get_art_container().Get(Cursor::DRAG_COPY_FRAME));
            }
            SetCursor(get_art_container().Get(Cursor::DRAG_FRAME));
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
        Canvas& active = m_app.GetActiveCanvas();
        Index index = GetFrameIndex(pos, active, false);
        active.RunCommand(remove_frame_command(index));
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
    int selected = GetSelectedFrame();
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
        SetCursor(get_art_container().Get(Cursor::DRAG_COPY_FRAME));
      }
    }
    else if (m_dragInfo.copy){
      SetCursor(get_art_container().Get(Cursor::DRAG_FRAME));
      m_dragInfo.copy = false;
    }
  }
  int GetDrawOffset(){
    wxSize sz = GetSize();
    if (m_bitmap.GetWidth() > sz.GetWidth()){
      int x = GetSelectedFrame() * (m_frameBoxSize.GetWidth() + iconSpacing);
      if (x > sz.GetWidth() / 2){
        return std::max(-x - (m_frameBoxSize.GetWidth() + iconSpacing) +
          sz.GetWidth() / 2,
          -m_bitmap.GetWidth() + sz.GetWidth());
      }
    }
    return 0;
  }

  int GetSelectedFrame(){
    Canvas& active = m_app.GetActiveCanvas();
    return active.GetSelectedFrame().Get();
  }

  Index GetFrameIndex(const IntPoint& pos, const Canvas& active, bool allowNext){
    // Gets the frame under pos
    Index numFrames(active.GetNumFrames());
    Index frame(std::max((pos.x - GetDrawOffset()) /
      (m_frameBoxSize.GetWidth() + iconSpacing), 0));
    if (frame < numFrames){
      return frame;
    }
    else{
      return allowNext ? numFrames : numFrames - 1;
    }
  }

  Index GetDropFrameIndex(const IntPoint& pos, const Canvas& active){
    // During drag and drop, the drop should be after the hovered frame
    // if the hovering is past its mid-point.
    // <../doc/frame-mid-point.png>
    const IntPoint offset(m_frameBoxSize.GetWidth() / 2, 0);
    return GetFrameIndex(pos + offset, active, true);
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

  AppContext& m_app;
  wxBitmap m_bitmap;
  wxBitmap m_closeFrameBitmap;
  wxBitmap m_closeFrameHighlightBitmap;
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
    AppContext& app,
    StatusInterface& status,
    const ArtContainer& art)
    : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize,
        wxTAB_TRAVERSAL|FRAMECTRL_BORDER_STYLE),
      m_app(app),
      m_listCtrl(nullptr)
  {
    wxBoxSizer* sizer = new wxBoxSizer(wxHORIZONTAL);

    // Fixme: Add binding to creation for noiseless_button (move to fwd-wx.hh)
    wxButton* addButton = noiseless_button(this,
      art.Get(Icon::ADD_FRAME),
      Tooltip("Add Frame"),
      wxSize(60,50));
    sizer->Add(addButton);

    m_listCtrl = new FrameListCtrl(this,
      art.Get(Icon::CLOSE_FRAME),
      art.Get(Icon::CLOSE_FRAME_HIGHLIGHT),
      app,
      status);
    sizer->Add(m_listCtrl);
    SetSizer(sizer);
    Layout();

    events::on_button(addButton,
      [this](){
        Canvas& canvas(m_app.GetActiveCanvas());
        canvas.RunCommand(add_frame_command(canvas.GetSize()));
      });
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
    const Index numFrames = m_app.GetActiveCanvas().GetNumFrames();
    if (GetShownFrames() == numFrames){
      m_listCtrl->Refresh();
      return false;
    }
    SetNumFrames(numFrames);
    return true;
  }

private:
  AppContext& m_app;
  FrameListCtrl* m_listCtrl;
};

FrameCtrl::FrameCtrl(wxWindow* parent, AppContext& app,
  StatusInterface& status,
  const ArtContainer& art)
{
  m_impl = new FrameCtrlImpl(parent, app, status, art);
}

FrameCtrl::~FrameCtrl(){
  m_impl = nullptr; // Deletion is handled by wxWidgets
}

wxWindow* FrameCtrl::AsWindow(){
  return m_impl;
}

bool FrameCtrl::Update(){
  return m_impl->UpdateFrames();
}

} // namespace
