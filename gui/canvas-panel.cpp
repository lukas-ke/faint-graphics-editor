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
#include "wx/dnd.h"
#include "wx/dcclient.h"
#include "app/app-context.hh"
#include "app/canvas-handle.hh"
#include "app/template-drawable.hh"
#include "geo/geo-func.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "gui/art.hh"
#include "gui/canvas-change-event.hh"
#include "gui/canvas-panel.hh"
#include "gui/canvas-panel-contexts.hh"
#include "rendering/paint-canvas.hh"
#include "tools/resize-canvas-tool.hh" // Fixme: Try to move to contexts
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-bind.hh"
#include "util-wx/key-codes.hh"
#include "util/convenience.hh"
#include "util/generator-adapter.hh"
#include "util/hit-test.hh"
#include "util/image.hh"
#include "util/image-util.hh"
#include "util/mouse.hh"
#include "util/object-util.hh"

namespace faint{

const ColRGB g_canvasBg(160, 140, 160);
const int g_lineSize = 10;

using CanvasChangeTag = const wxEventTypeTag<CanvasChangeEvent>;
extern CanvasChangeTag EVT_FAINT_GRID_CHANGE;
extern CanvasChangeTag EVT_FAINT_ZOOM_CHANGE;

const wxEventType FAINT_CANVAS_CHANGE = wxNewEventType();
CanvasChangeTag EVT_FAINT_CANVAS_CHANGE(FAINT_CANVAS_CHANGE);

const wxEventType FAINT_GRID_CHANGE = wxNewEventType();
CanvasChangeTag EVT_FAINT_GRID_CHANGE(FAINT_GRID_CHANGE);

const wxEventType FAINT_ZOOM_CHANGE = wxNewEventType();
CanvasChangeTag EVT_FAINT_ZOOM_CHANGE(FAINT_ZOOM_CHANGE);

const long g_panel_style = static_cast<long>(wxVSCROLL|wxHSCROLL | wxWANTS_CHARS);

const int objectHandleWidth = 8;

static bool is_tool_modifier(int keycode){
  return keycode == key::ctrl || keycode == key::shift;
}

static KeyPress char_event_to_keypress(const wxKeyEvent& event,
  bool& probablyCtrlEnter)
{
  int keyCode = event.GetKeyCode();
  int modifiers = event.GetModifiers();

  if ((modifiers & wxMOD_CONTROL) != 0){
    // Untranslate Ctrl+Letter keycode (See \ref(ctrl-enter-workaround)).
    if (keyCode <= 26){
      if (keyCode == 10){
        keyCode = probablyCtrlEnter ? key::enter : key::J;
      }
      else{
        // Fixme: Explain
        keyCode = 'A' + keyCode - 1;
      }
    }
  }

  // The workaround (see wxEVT_KEY_DOWN handler) should only survive through a
  // single character event (wxEVT_CHAR).
  probablyCtrlEnter = false;

  return KeyPress(Mod::Create(modifiers), Key(keyCode),
    to_faint(event.GetUnicodeKey()));
}

CanvasPanel::CanvasPanel(wxWindow* parent,
  ImageList&& images,
  const initially_dirty& startDirty,
  wxFileDropTarget* fileDropTarget,
  const Art& art,
  AppContext& app,
  StatusInterface& statusInfo)
  : wxPanel(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, g_panel_style),
    m_art(art),
    m_contexts(app),
    m_images(std::move(images)),
    m_mouse(this, OnLoss([=](){Preempt(PreemptOption::ALLOW_COMMAND);})),
    m_statusInfo(statusInfo)
{
  bind_fwd(this, wxEVT_CHAR,
    [this](wxKeyEvent& event){
      KeyInfo info(GetInterface(),
        m_statusInfo,
        m_contexts.app.GetLayerType(),
        char_event_to_keypress(event, m_probablyCtrlEnter));

      if (info.key.Alt() && !info.key.Ctrl()){
        // Do not allow the tool to handle key-presses involving Alt as
        // this could prevent opening menus with Alt (e.g. Alt+F).
        //
        // \def(alt-xubuntu)Alt-problem; ...however, let Alt slip
        // through if Ctrl is held, as in xubuntu, at least in
        // Virtualbox 4.3.10, Alt+Gr was interpreted as Ctrl+Alt. If
        // this doesn't reach the tool some keys, like backslash,
        // can't be typed (at least not with Swedish keyboard).
        event.Skip();
      }
      else if (HandleToolResult(m_contexts.GetTool().Char(info))) {
        // The tool ate the character, don't propagate.
        return;
      }
      else{
        // Key ignored by tool. Allow normal key handling.
        event.Skip();
      }
    });

  bind_fwd(this, wxEVT_LEFT_DCLICK,
    [&](const wxMouseEvent& event){
      PosInfo info = HitTest(to_faint(event.GetPosition()),
        mouse_modifiers(event));
      HandleToolResult(m_contexts.GetTool().DoubleClick(info));
    });

  events::no_op_erase_background(this);

  events::on_idle(this, [this]() {
    // Update the scrollbars in an idle handler to prevent various
    // crashes and mouse capture problems
    // (e.g. issue 78)
    if (neither(m_scroll.updateHorizontal, m_scroll.updateVertical)){
      return;
    }
    if (then_false(m_scroll.updateHorizontal)){
      AdjustHorizontalScrollbar(m_state.geo.pos.x);
    }
    if (then_false(m_scroll.updateVertical)){
      AdjustVerticalScrollbar(m_state.geo.pos.y);
    }
    Refresh();
  });

  bind_fwd(this, wxEVT_KEY_DOWN,
    [this](wxKeyEvent& event){
      int keycode = event.GetKeyCode();
      if (is_tool_modifier(keycode)){
        // Update the tool state if a modifier is pressed (e.g. for cursor update)
        // Note: This repeats while the key is held.
        MousePosRefresh();
        return;
      }

      // \def(ctrl-enter-workaround)Workaround;
      // It is not possible to discern between Ctrl+Enter and Ctrl+J in
      // the OnChar-handler.
      m_probablyCtrlEnter = event.GetKeyCode() == WXK_RETURN &&
        (event.GetModifiers() & wxMOD_CONTROL);
      event.Skip();
    });

  bind_fwd(this, wxEVT_KEY_UP,
    [this](wxKeyEvent& event){
      int keycode = event.GetKeyCode();
      if (is_tool_modifier(keycode)){
        // Update the tool state if a modifier is released (e.g. for
        // cursor update) Note: This repeates while the key is held.
        MousePosRefresh();
        return;
      }
      event.Skip();
    });

  bind_fwd(this, wxEVT_LEFT_DOWN, wxEVT_RIGHT_DOWN,
    [this](wxMouseEvent& evt){
      SetFocus();
      IntPoint viewPos(to_faint(evt.GetPosition()));
      PosInfo info(HitTest(viewPos, mouse_modifiers(evt)));
      m_mouse.Capture();

      if (!IgnoreCanvasHandle(info)){
        // Switch to canvas resize tool if a canvas handle was clicked.
        IntSize imageSize(m_images.Active().GetSize());

        if (auto handle = canvas_handle_hit_test(viewPos, imageSize, m_state.geo)){
          Settings toolSettings(m_contexts.app.GetToolSettings());
          Tool* resizeTool = evt.GetButton() == wxMOUSE_BTN_LEFT ?
            resize_canvas_tool(handle.Get(), toolSettings) :
            scale_canvas_tool(handle.Get(), toolSettings);
          m_contexts.tool.SetSwitched(resizeTool);
          SetFaintCursor(handle.Get().GetCursor());
          return;
        }
      }
      HandleToolResult(m_contexts.GetTool().MouseDown(info));
      SetFaintCursor(m_contexts.GetTool().GetCursor(info));

      // Notify app to allow in-tool undo (e.g. Polygon tool point adding).
      SendCanvasChangeEvent();
    });

  bind_fwd(this, wxEVT_LEFT_UP, wxEVT_RIGHT_UP,
    [this](wxMouseEvent& evt){
      m_mouse.Release();
      SetFocus();
      PosInfo info = HitTest(to_faint(evt.GetPosition()), mouse_modifiers(evt));
      HandleToolResult(m_contexts.GetTool().MouseUp(info));
  });

  events::on_mouse_motion(this, [&](const IntPoint& pos){
      MousePosRefresh(pos, get_tool_modifiers());
    });

  events::on_mouse_leave_window(this, [&](){
    if (m_contexts.GetTool().RefreshOnMouseOut()){
      // Clear graphics remains, see \ref(refresh-on-mouse-out).
      RefreshToolRect();
    }
    m_statusInfo.Clear();
  });

  bind_fwd(this, wxEVT_MOUSEWHEEL,
    [this](wxMouseEvent& event){
      int rot = event.GetWheelRotation();
      if (rot > 0){
        if (event.ControlDown()){
          ScrollLineLeft();
        }
        else{
          ScrollLineUp();
        }
      }
      else if (rot < 0){
        if (event.ControlDown()){
          ScrollLineRight();
        }
        else{
          ScrollLineDown();
        }
      }
    });

  events::on_paint(this, [&](){
    auto selectionMirage = m_mirage.selection.lock();
    const RasterSelection& rasterSelection(selectionMirage == nullptr ?
      GetImageSelection() : *selectionMirage);

    wxPaintDC dc(this);
    auto layer = m_contexts.app.GetLayerType();
    paint_canvas(dc,
      m_images.Active(),
      template_drawable(m_images.Active().GetObjects()),
      m_state,
      m_images.GetGrid(),
      to_faint(GetUpdateRegion().GetBox()),
      m_mirage.bitmap,
      g_canvasBg,
      HitTest(mouse::view_position(*this), get_tool_modifiers()),
      rasterSelection,
      template_drawable(m_contexts.tool.GetTool()),
      m_contexts.app.GetTransparencyStyle(),
      layer,
      objectHandleWidth,
      template_drawable(m_contexts.app.GetExtraOverlay()));
  });

  bind_fwd(this, wxEVT_SCROLLWIN_THUMBTRACK,
    // This is a recurring event while a scrollbar-thumb is dragged
    [this](wxScrollWinEvent& event){
      event.Skip();
      auto& geo = m_state.geo;
      int orientation = event.GetOrientation();
      int pos = event.GetPosition();
      if (orientation == wxHORIZONTAL){
        geo.pos.x = pos + m_scroll.startX;
      }
      else if (orientation == wxVERTICAL){
        geo.pos.y = pos + m_scroll.startY;
      }
      Refresh();
    });

  bind_fwd(this, wxEVT_SCROLLWIN_THUMBRELEASE,
    [this](wxScrollWinEvent& event){
      auto& geo = m_state.geo;
      event.Skip();
      if (event.GetOrientation() == wxHORIZONTAL){
        AdjustHorizontalScrollbar(geo.pos.x);
      }
      else {
        AdjustVerticalScrollbar(geo.pos.y);
      }
    });

  bind_fwd(this, wxEVT_SCROLLWIN_LINEDOWN,
    [this](wxScrollWinEvent& event){
      event.Skip();
      const int orientation = event.GetOrientation();
      if (orientation == wxHORIZONTAL){
        ScrollLineRight();
      }
      else if (orientation == wxVERTICAL){
        ScrollLineDown();
      }
      Refresh();
    });

  bind_fwd(this, wxEVT_SCROLLWIN_LINEUP,
    [this](wxScrollWinEvent& event){
      event.Skip();
      const int orientation = event.GetOrientation();
      if (orientation == wxHORIZONTAL){
        ScrollLineLeft();
      }
      else if (orientation == wxVERTICAL){
        ScrollLineUp();
      }
      Refresh();
    });


  bind_fwd(this, wxEVT_SCROLLWIN_PAGEDOWN,
    [this](wxScrollWinEvent& event){
      event.Skip();
      if (event.GetOrientation() == wxVERTICAL){
        ScrollPageDown();
      }
      else if (event.GetOrientation() == wxHORIZONTAL){
        ScrollPageRight();
      }
    });

  bind_fwd(this, wxEVT_SCROLLWIN_PAGEUP,
    [this](wxScrollWinEvent& event){
      event.Skip();
      if (event.GetOrientation() == wxVERTICAL){
        ScrollPageUp();
      }
      else if (event.GetOrientation() == wxHORIZONTAL){
        ScrollPageLeft();
      }
    });

  // Fixme: Fails under Gtk/X due to "window not yet created"
  #ifdef __WXMSW__
  bind_fwd(this, wxEVT_SIZE, [&](wxSizeEvent& event){
      AdjustScrollbars(m_state.geo.Pos());
      RefreshToolRect();
      event.Skip();
  });
  #endif

  SetBackgroundColour(to_wx(g_canvasBg));
  SetBackgroundStyle(wxBG_STYLE_PAINT);

  m_contexts.canvas = create_canvas_context(*this, m_contexts.app, m_images);
  m_contexts.command = create_command_context(*this, m_images);

  SetDropTarget(fileDropTarget);

  if (startDirty.Get()){
    // Set a unique command id so that the image remains dirty until
    // the first save.
    m_document.savedAfter.Set(CommandId());
  }
}

bool CanvasPanel::AcceptsFocus() const{
  return true;
}

void CanvasPanel::AdjustHorizontalScrollbar(int pos){
  auto& geo = m_state.geo;
  geo.pos.x = pos;
  pos = std::max(0,pos);
  m_scroll.startX = geo.pos.x < 0 ? geo.pos.x : 0;
  SetScrollbar(wxHORIZONTAL, // Orientation
    pos, // Position
    GetHorizontalPageSize(), // Thumb size
    std::max(GetMaxScrollRight(), pos + GetHorizontalPageSize() - m_scroll.startX), // Range
    true); // Refresh
}

void CanvasPanel::AdjustScrollbars(const IntPoint& p){
  AdjustHorizontalScrollbar(p.x);
  AdjustVerticalScrollbar(p.y);
}

void CanvasPanel::AdjustVerticalScrollbar(int pos){
  auto& geo = m_state.geo;
  geo.pos.y = pos;
  pos = std::max(0, pos);
  m_scroll.startY = geo.pos.y < 0 ? geo.pos.y : 0;
  const auto range = std::max(GetMaxScrollDown(),
    pos + GetVerticalPageSize() - m_scroll.startY);
  SetScrollbar(wxVERTICAL, // Orientation
    pos, // Position
    GetVerticalPageSize(), // Thumb size
    range,
    true); // Refresh
}

bool CanvasPanel::CanRedo() const{
  return m_contexts.GetTool().HistoryContext().Visit(
    [&](const faint::HistoryContext& c){
      return c.CanRedo() || (c.AllowsGlobalRedo() && m_commands.CanRedo());
    },
    [&](){
      return m_commands.CanRedo();
    });
}

bool CanvasPanel::CanUndo() const{
  return m_commands.CanUndo() ||
    m_contexts.GetTool().HistoryContext().Visit(
      [&](const faint::HistoryContext& c){
        return c.CanUndo();
      },
    [&](){
      return false;
    });
}

void CanvasPanel::CenterView(const IntPoint& ptView){
  auto& geo = m_state.geo;
  IntPoint viewCenter = point_from_size(to_faint(GetClientSize())) / 2;
  IntPoint pos = ptView + geo.pos - viewCenter;
  if (pos == geo.pos){
    // Do nothing if the ptView refers to the currently centered image point, as
    // this would make it difficult to move the cursor while
    // continuously centering at large zoom.
    return;
  }
  geo.pos = pos;
  AdjustScrollbars(geo.pos);
  WarpPointer(viewCenter.x, viewCenter.y);
  MousePosRefresh();
  Refresh();
}

void CanvasPanel::CenterViewImage(const Point& ptImage){
  CenterView(mouse::image_to_view(floored(ptImage), m_state.geo));
}

void CanvasPanel::ChangeZoom(ZoomLevel::ChangeType changeType){
  auto& geo = m_state.geo;
  coord oldScale = geo.Scale();
  geo.zoom.Change(changeType);
  geo.pos = truncated(geo.pos / (oldScale / geo.Scale()));
  AdjustScrollbars(geo.pos);
  MousePosRefresh();
  SendZoomChangeEvent();
  Refresh();
}

void CanvasPanel::ClearPointOverlay(){
  m_state.pointOverlay.Clear();
}

void CanvasPanel::DeselectObject(Object* obj){
  DeselectObjects(as_list(obj));
}

void CanvasPanel::DeselectObjects(){
  m_images.Active().DeselectObjects();
  m_contexts.GetTool().SelectionChange();
  SendCanvasChangeEvent();
}

void CanvasPanel::DeselectObjects(const objects_t& objects){
  bool deselected = m_images.Active().Deselect(objects);
  if (!deselected){
    return;
  }
  m_contexts.GetTool().SelectionChange();
  SendCanvasChangeEvent();
}

PosInfo CanvasPanel::ExternalHitTest(const IntPoint& ptImage){
  IntPoint ptView = mouse::image_to_view(ptImage, m_state.geo);
  return HitTest(ptView, get_tool_modifiers());
}

IntPoint CanvasPanel::GetFaintScrollPos() const{
  return m_state.geo.Pos();
}

CanvasId CanvasPanel::GetCanvasId() const{
  return m_canvasId;
}

Optional<FilePath> CanvasPanel::GetFilePath() const{
  return m_document.filename;
}

Grid CanvasPanel::GetGrid() const{
  return m_images.GetGrid();
}

const ImageList& CanvasPanel::GetImageList() const{
  return m_images;
}

Point CanvasPanel::GetImageViewStart() const{
  auto& geo = m_state.geo;
  const Point pos = (geo.pos - point_from_size(geo.border)) / geo.Scale();
  return max_coords(pos, Point::Both(0));
}

RasterSelection& CanvasPanel::GetImageSelection(){
  return m_images.Active().GetRasterSelection();
}

const RasterSelection& CanvasPanel::GetImageSelection() const{
  return m_images.Active().GetRasterSelection();
}

Canvas& CanvasPanel::GetInterface(){
  return *m_contexts.canvas;
}

int CanvasPanel::GetMaxScrollRight() const{
  auto scale = m_state.geo.Scale();
  auto w = m_images.Active().GetSize().w;
  return floored(std::max(0.0, w * scale));
}

int CanvasPanel::GetMaxScrollDown() const{
  auto scale =  m_state.geo.Scale();
  auto h = m_images.Active().GetSize().h;
  return floored(std::max(0.0, h * scale));
}

IntPoint CanvasPanel::GetMaxUsefulScroll() const{
  return IntPoint(GetMaxScrollRight(), GetMaxScrollDown()) -
    IntPoint(GetHorizontalPageSize(), GetVerticalPageSize());
}

MenuFlags CanvasPanel::GetMenuFlags() const{
  const Image& active(m_images.Active());
  bool shouldDrawRaster = should_draw_raster(m_contexts.tool,
    m_contexts.app.GetLayerType());
  bool shouldDrawVector = should_draw_vector(m_contexts.tool,
    m_contexts.app.GetLayerType());

  MenuFlags fl;
  fl.toolSelection = HasToolSelection();
  fl.rasterSelection = shouldDrawRaster && HasRasterSelection();
  fl.objectSelection = shouldDrawVector && HasObjectSelection();
  fl.numSelected = active.GetObjectSelection().size();
  fl.groupIsSelected = contains_group(active.GetObjectSelection());
  fl.hasObjects = active.GetNumObjects() != 0;
  fl.dirty = IsDirty();
  fl.canUndo = CanUndo();
  fl.canRedo = CanRedo();
  fl.undoLabel = GetUndoName();
  fl.redoLabel = GetRedoName();
  fl.canMoveForward = can_move_forward(active, active.GetObjectSelection());
  fl.canMoveBackward = can_move_backward(active, active.GetObjectSelection());
  return fl;
}

const objects_t& CanvasPanel::GetObjects(){
  return m_images.Active().GetObjects();
}

utf8_string CanvasPanel::GetRedoName() const{
  return m_contexts.GetTool().HistoryContext().Visit(
    [&](const HistoryContext& c) -> utf8_string{
      if (c.CanRedo()){
        return c.GetRedoName();
      }
      else if (c.PreventsGlobalRedo()){
        return "";
      }
      return m_commands.GetRedoName(m_images);
    },
    [&](){
      return m_commands.GetRedoName(m_images);
    });
}

Point CanvasPanel::GetRelativeMousePos(){
  return mouse::image_position(m_state.geo, *this);
}

Index CanvasPanel::GetSelectedFrame() const{
  return m_images.GetActiveIndex();
}

const objects_t& CanvasPanel::GetObjectSelection(){
  return m_images.Active().GetObjectSelection();
}

utf8_string CanvasPanel::GetUndoName() const{
  return m_contexts.GetTool().HistoryContext().Visit(
    [&](const HistoryContext& c) -> utf8_string{
      return c.CanUndo() ?
        c.GetUndoName() : m_commands.GetUndoName(m_images);
    },
    [&](){
      return m_commands.GetUndoName(m_images);
    });
}

coord CanvasPanel::GetZoom() const{
  return m_state.geo.zoom.GetScaleFactor();
}

const ZoomLevel& CanvasPanel::GetZoomLevel() const{
  return m_state.geo.zoom;
}

bool CanvasPanel::Has(const ObjectId& id){
  const objects_t& objects(m_images.Active().GetObjects());
    return any_of(objects,
      [&](auto obj){
        return is_or_has(obj, id);
      });
}

bool CanvasPanel::HasObjectSelection() const{
  return !m_images.Active().GetObjectSelection().empty();
}

bool CanvasPanel::HasRasterSelection() const{
  return GetImageSelection().Exists();
}

bool CanvasPanel::HasSelection() const{
  return HasRasterSelection() || HasObjectSelection();
}

bool CanvasPanel::HasToolSelection() const{
  return m_contexts.GetTool().SelectionContext().IsSet();
}

bool CanvasPanel::InSelection(const Point& p){
  return GetImageSelection().Contains(floored(p));
}

bool CanvasPanel::IsDirty() const{
  const Optional<CommandId> lastModifying(m_commands.GetLastModifying());
  if (lastModifying.NotSet()){
    // No remaining undoable commands modified the image.
    // If the image was saved during this session, we've undone past
    // that point (and the image is dirty).
    // If the image has not been saved, we've undone back to the load
    // state, and the image is not dirty.
    return m_document.savedAfter.IsSet();
  }

  // There are undoable modifying commands, and the image hasn't been
  // saved during this session or was saved earlier or later.
  return m_document.savedAfter.NotSet() ||
    (lastModifying != m_document.savedAfter.Get());
}

void CanvasPanel::MousePosRefresh(){
  MousePosRefresh(mouse::view_position(*this), get_tool_modifiers());
}

PreemptResult CanvasPanel::Preempt(PreemptOption option){
  Tool& tool = m_contexts.GetTool();

  PosInfo info(HitTest(mouse::view_position(*this),
    get_tool_modifiers()));
  ToolResult r = tool.Preempt(info);
  if (r == ToolResult::COMMIT){
    if (option == PreemptOption::ALLOW_COMMAND){
      CommitTool(tool, REFRESH);
    }
    return PreemptResult::COMMIT;
  }
  else if (r == ToolResult::CANCEL){
    Refresh();
    return PreemptResult::CANCEL;
  }
  else if (r == ToolResult::CHANGE){
    if (option == PreemptOption::ALLOW_COMMAND){
      CommitTool(tool, REFRESH);
    }
    m_contexts.tool.ClearSwitched();
    return PreemptResult::COMMIT;
  }
  else {
    Refresh();
    return PreemptResult::NONE;
  }
}

void CanvasPanel::PreviousFrame(){
  const Index maxIndex = m_images.GetNumImages();
  const Index oldIndex = m_images.GetActiveIndex();
  const Index index(oldIndex.Get() == 0 ? maxIndex - 1 : oldIndex - 1);
  SelectFrame(index);
}

void CanvasPanel::NextFrame(){
  const Index maxIndex = m_images.GetNumImages();
  const Index oldIndex = m_images.GetActiveIndex();
  const Index index((oldIndex.Get() + 1) % maxIndex.Get());
  SelectFrame(index);
}

void CanvasPanel::NotifySaved(const FilePath& path){
  m_document.filename.Set(path);
  m_document.savedAfter = m_commands.GetLastModifying();
  SendCanvasChangeEvent();
}

void CanvasPanel::Redo(){

  auto toolUndo =
    [&](Tool& tool){
    return tool.HistoryContext().Visit(
      [&](faint::HistoryContext& c){
        if (c.CanRedo()){
          c.Redo();
          MousePosRefresh();
          // For undo/redo state
          SendCanvasChangeEvent();
          return true;
        }
        else{
          return c.PreventsGlobalRedo();
        }
      },
      [&](){
        return false;
      });
  };
  if (toolUndo(m_contexts.GetTool())){
    return;
  }
  else if (m_commands.CanRedo()){
    // Do not allow the preempted tool to commit, as this would
    // mess up undo/redo.

    Preempt(PreemptOption::DISCARD_COMMAND);

    m_commands.Redo(*m_contexts.command, m_state.geo, m_images);

    // For undo/redo state.
    SendCanvasChangeEvent();
  }
}

void CanvasPanel::RunCommand(Command* cmd){
  // When a command is run, any commands in the redo list must be
  // cleared (See exception in CanvasPanel::Redo).
  RunCommand(cmd, clear_redo(true), nullptr);
}

void CanvasPanel::RunCommand(Command* cmd, const FrameId& frameId){
  // When a command is run, any commands in the redo list must be
  // cleared (See exception in CanvasPanel::Redo).
  RunCommand(cmd, clear_redo(true), &m_images.GetImage(frameId));
}

void CanvasPanel::RunDWIM(){
  if (m_commands.ApplyDWIM(m_images, *m_contexts.command, m_state.geo)){
    Refresh();
  }
}

void CanvasPanel::ScrollMaxDown(){
  m_state.geo.pos.y = std::max(0, GetMaxScrollDown() - GetVerticalPageSize());
  AdjustScrollbars(m_state.geo.Pos());
  Refresh();
}

void CanvasPanel::ScrollMaxLeft(){
  m_state.geo.pos.x = 0;
  AdjustScrollbars(m_state.geo.Pos());
  Refresh();
}

void CanvasPanel::ScrollMaxRight(){
  m_state.geo.pos.x = std::max(0, GetMaxScrollRight() - GetHorizontalPageSize());
  AdjustScrollbars(m_state.geo.Pos());
  Refresh();
}

void CanvasPanel::ScrollMaxUp(){
  m_state.geo.pos.y = 0;
  AdjustScrollbars(m_state.geo.Pos());
  Refresh();
}

void CanvasPanel::ScrollPageDown(){
  auto& geo = m_state.geo;
  const int pageSize = GetVerticalPageSize();
  const int y = geo.pos.y + pageSize;
  geo.pos.y = std::max(0, std::min(y, GetMaxScrollDown() - pageSize));
  m_scroll.updateVertical = true;
}

void CanvasPanel::ScrollPageLeft(){
  auto& geo = m_state.geo;
  geo.pos.x = std::max(0, geo.pos.x - GetHorizontalPageSize());
  m_scroll.updateHorizontal = true;
}

void CanvasPanel::ScrollPageRight(){
  auto& geo = m_state.geo;
  const int pageSize = GetHorizontalPageSize();
  const int x = geo.pos.x + pageSize;
  geo.pos.x = std::max(0, std::min(x, GetMaxScrollRight() - pageSize));
  m_scroll.updateHorizontal = true;
}

void CanvasPanel::ScrollPageUp(){
  auto& geo = m_state.geo;
  geo.pos.y = std::max(0, geo.pos.y - GetVerticalPageSize());
  m_scroll.updateVertical = true;
}

void CanvasPanel::SelectObject(Object* obj, const deselect_old& deselectOld){
  SelectObjects(as_list(obj), deselectOld);
}

void CanvasPanel::SelectObjects(const objects_t& objects,
  const deselect_old& deselectOld)
{
  if (deselectOld.Get()){
    m_images.Active().DeselectObjects();
  }
  m_images.Active().SelectObjects(objects);
  m_contexts.GetTool().SelectionChange();
  SendCanvasChangeEvent();
  Refresh();
}

void CanvasPanel::SendCanvasChangeEvent(){
  CanvasChangeEvent event(FAINT_CANVAS_CHANGE, GetCanvasId());
  ProcessEvent(event);
}

void CanvasPanel::SendGridChangeEvent(){
  CanvasChangeEvent event(FAINT_GRID_CHANGE, GetCanvasId());
  ProcessEvent(event);
}

void CanvasPanel::SendZoomChangeEvent(){
  CanvasChangeEvent event(FAINT_ZOOM_CHANGE, GetCanvasId());
  ProcessEvent(event);
}

void CanvasPanel::SetFaintScrollPos(const IntPoint& pos){
  m_state.geo.SetPos(pos);
  m_scroll.updateHorizontal = true;
  m_scroll.updateVertical = true;
}

void CanvasPanel::SetGrid(const Grid& g){
  m_images.SetGrid(g);
  SendGridChangeEvent();
}

void CanvasPanel::SetMirage(const std::weak_ptr<Bitmap>& bitmap){
  m_mirage.bitmap = bitmap;
  Refresh();
}

void CanvasPanel::SetMirage(const std::weak_ptr<RasterSelection>& selection){
  m_mirage.selection = selection;
  Refresh();
}

void CanvasPanel::SelectFrame(const Index& index){
  assert(index < m_images.GetNumImages());
  if (index != m_images.GetActiveIndex()){
    Preempt(PreemptOption::ALLOW_COMMAND);
    m_images.SetActiveIndex(index);
    m_contexts.command->SetFrame(&m_images.Active());
    m_contexts.GetTool().SelectionChange();

    // For frame control
    SendCanvasChangeEvent();
    Refresh();
  }
}

void CanvasPanel::SetPointOverlay(const IntPoint& p){
  m_state.pointOverlay.Set(p);
}

void CanvasPanel::SetZoomLevel(const ZoomLevel& zoom){
  m_state.geo.zoom = zoom;
  SendCanvasChangeEvent();
  MousePosRefresh();
}

void CanvasPanel::Undo(){
  Tool& tool = m_contexts.GetTool();
  if (tool.HistoryContext().Visit(
    [&](HistoryContext& c){
      if (c.CanUndo()){
        c.Undo();
        MousePosRefresh();
        // For undo/redo-state
        SendCanvasChangeEvent();
        return true;
      }
      return false;
    },
    [](){
      return false;
    }))
  {
    return;
  }

  // Preempt the current tool, but allow it to commit.
  // if committing, whatever it did will be undone.
  PreemptResult res = Preempt(PreemptOption::ALLOW_COMMAND);
  if (res == PreemptResult::CANCEL){
    // The tool was aborted by the undo
    return;
  }

  // The tool either committed on the preempt (and the command should
  // now be undone) or there was no effect. Proceed with normal
  // undo-behavior.
  if (m_commands.Undo(*m_contexts.command, m_state.geo)){
    SendCanvasChangeEvent();

    // Update the selection settings etc (originally added to make the
    // ObjSelectTool change the controls if a setting change was
    // undone)
    m_contexts.GetTool().SelectionChange();
  }
}

void CanvasPanel::ZoomFit(){
  auto& geo = m_state.geo;

  // Use the complete size rather than the client size, so that
  // scrollbars don't limit the available size
  const IntSize windowSize(to_faint(GetSize()));
  const IntSize borders(geo.border * 2);
  const Size available(floated(max_coords(windowSize - borders, IntSize(1,1))));
  const Size imageSize(floated(m_images.Active().GetSize()));
  Scale rel(NewSize(available), imageSize);
  geo.zoom.SetApproximate(std::min(rel.x, rel.y));
  AdjustScrollbars(geo.pos = {0,0});
  MousePosRefresh();
  SendZoomChangeEvent();
  Refresh();
}

int CanvasPanel::GetHorizontalPageSize() const {
  return GetClientSize().GetWidth() - m_state.geo.border.w * 2;
}

int CanvasPanel::GetVerticalPageSize() const {
  return GetClientSize().GetHeight() - m_state.geo.border.h * 2;
}

Optional<IntPoint> CanvasPanel::GetPointOverlay() const{
  return m_state.pointOverlay;
}

bool CanvasPanel::IgnoreCanvasHandle(const PosInfo& info){
  // Prefer a clicked object handle with object selection tool
  // over canvas resize-handles.
  return info.handle.IsSet() &&
    m_contexts.tool.GetToolId() == ToolId::SELECTION &&
    info.layerType == Layer::RASTER;
}

void CanvasPanel::MousePosRefresh(const IntPoint& viewPt,
  const ToolModifiers& modifiers)
{
  PosInfo info = HitTest(viewPt, modifiers);
  if (!m_mouse.HasCapture() && !IgnoreCanvasHandle(info)){
    auto canvasHandle = canvas_handle_hit_test(viewPt,
      m_images.Active().GetSize(), m_state.geo);

    if (canvasHandle.IsSet()){
      SetFaintCursor(canvasHandle.Get().GetCursor());
      m_statusInfo.SetMainText("Left click to resize, right click to rescale.");
      return;
    }
  }

  if (!m_contexts.app.GetInteraction().MouseMove(info)){
    // Todo: Tidy up.
    auto& tool(m_contexts.GetTool());
    HandleToolResult(tool.MouseMove(info));
    SetFaintCursor(tool.GetCursor(info));
  }
}

void CanvasPanel::ScrollLineUp(){
  auto& geo = m_state.geo;
  AdjustVerticalScrollbar(geo.pos.y = std::max(0, geo.pos.y - g_lineSize));
}

void CanvasPanel::ScrollLineDown(){
  auto& geo = m_state.geo;
  const auto maxScroll = GetMaxScrollDown() - GetVerticalPageSize();
  AdjustVerticalScrollbar(geo.pos.y =
    std::min(maxScroll, geo.pos.y + g_lineSize));
}

void CanvasPanel::ScrollLineLeft(){
  auto& geo = m_state.geo;
  AdjustHorizontalScrollbar(geo.pos.x = std::max(0, geo.pos.x - g_lineSize));
}

void CanvasPanel::ScrollLineRight(){
  auto& geo = m_state.geo;
  const auto maxScroll = GetMaxScrollRight() - GetHorizontalPageSize();
  AdjustHorizontalScrollbar(geo.pos.x =
    std::min(maxScroll, geo.pos.x + g_lineSize));
}

static IntRect get_visible_rect(const IntSize& size, const CanvasGeo& geo){
  const coord zoom = geo.Scale();
  return IntRect(floored((geo.pos - point_from_size(geo.border)) / zoom),
    truncated(size / zoom + Size::Both(100)));
}

void CanvasPanel::RefreshToolRect(){
  Tool& tool = m_contexts.GetTool();
  const RefreshInfo info(get_visible_rect(to_faint(GetSize()), m_state.geo),
    mouse::image_position(m_state.geo, *this), objectHandleWidth);

  const IntRect toolRect(tool.GetRefreshRect(info));
  InclusiveRefresh(mouse::image_to_view(toolRect, m_state.geo));
}

void CanvasPanel::CommitTool(Tool& tool, RefreshMode refreshMode){
  Command* cmd = tool.GetCommand();
  if (cmd == nullptr){
    return;
  }

  RunCommand(cmd);
  if (refreshMode == REFRESH){
    Refresh();
  }
}

void CanvasPanel::CloseUndoBundle(const utf8_string& name){
  m_commands.CloseUndoBundle(name);
  SendCanvasChangeEvent();
}

PosInfo CanvasPanel::HitTest(const IntPoint& ptView, const ToolModifiers& mod){

  ObjectInfo objInfo = hit_test(ptView,
    m_images.Active(),
    m_state.geo, objectHandleWidth);

  Point ptImage = mouse::view_to_image(ptView, m_state.geo);
  PosInfo info = {
    GetInterface(),
    m_statusInfo,
    mod,
    ptImage,
    m_contexts.app.TabletGetCursor(),
    InSelection(ptImage),
    m_contexts.app.GetLayerType(),
    objInfo};
  return info;
}

void CanvasPanel::OpenUndoBundle(){
  m_commands.OpenUndoBundle();
}

void CanvasPanel::SetFaintCursor(Cursor cursor){
  if (cursor == Cursor::DONT_CARE){
    return;
  }
  SetCursor(m_art.Get(cursor));
}

void CanvasPanel::RunCommand(Command* cmd,
  const clear_redo& clearRedo,
  Image* targetFrame)
{
  if (targetFrame == nullptr){
    targetFrame = &(m_images.Active());
  }

  Optional<IntPoint> offset = m_commands.Apply(cmd,
    clearRedo,
    targetFrame,
    m_images,
    *m_contexts.command,
    m_state.geo);

  if (offset.IsSet()){
    AdjustScrollbars(offset.Get());
  }

  if (!m_commands.Bundling()){
    SendCanvasChangeEvent();
  }
  m_contexts.GetTool().SelectionChange();
}

// Refreshes the union of the rectangle parameter and the rectangle
// parameter from the previous call.
void CanvasPanel::InclusiveRefresh(const IntRect& r){
  const coord zoom = m_state.geo.zoom.GetScaleFactor();
  if (zoom >= 1.0){
    RefreshRect(to_wx(union_of(r, m_lastRefreshRect)).
      Inflate(floored(2.0 * zoom)));
    m_lastRefreshRect = r;
  }
  else{
    // Fixme - Workaround. RefreshRect does not work well when zoomed out,
    // leading to problems with offsets and other stuff.
    // Note: This is slow.
    Refresh();
  }
}

bool CanvasPanel::HandleToolResult(ToolResult ref){
  if (ref == ToolResult::COMMIT){
    CommitTool(m_contexts.GetTool(), REFRESH);
  }
  else if (ref == ToolResult::CHANGE){
    CommitTool(m_contexts.GetTool(), NO_REFRESH);
    m_contexts.tool.ClearSwitched();

    // Update settings, e.g. when cloning objects from the
    // ObjSelectTool
    m_contexts.GetTool().SelectionChange();
    Refresh();
  }
  else if (ref == ToolResult::DRAW){
    RefreshToolRect();
  }
  else if (ref == ToolResult::CANCEL){
    Refresh();
  }
  else if (ref == ToolResult::SETTING_CHANGED){
    RefreshToolRect();
    SendCanvasChangeEvent();
  }
  // ToolResult::NONE might signify that the tool didn't care, which can in
  // some cases suggest further processing outside this function.
  return ref != ToolResult::NONE;
}

} // namespace

namespace faint{ namespace events{

static void on_canvas_event(wxWindow* w,
  const CanvasChangeTag& tag,
  const std::function<void(CanvasId)>& f,
  bool skip)
{
  bind_fwd(w, tag,
    [f, skip](CanvasChangeEvent& e){
      f(e.GetCanvasId());
      if (skip){
        e.Skip();
      }
    });
}

void on_canvas_modified(window_t w, const std::function<void(CanvasId)>& f){
  on_canvas_event(w.w, EVT_FAINT_CANVAS_CHANGE, f, false);
}

void on_canvas_modified_skip(window_t w, const std::function<void(CanvasId)>& f){
  on_canvas_event(w.w, EVT_FAINT_CANVAS_CHANGE, f, true);
}

void on_grid_modified(window_t w, canvas_id_fn f){
  on_canvas_event(w.w, EVT_FAINT_GRID_CHANGE, f, false);
}

void on_zoom_modified(window_t w, canvas_id_fn f){
  on_canvas_event(w.w, EVT_FAINT_ZOOM_CHANGE, f, false);
}

}} // namespace
