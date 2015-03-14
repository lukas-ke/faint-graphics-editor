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

#ifndef FAINT_CANVAS_PANEL_HH
#define FAINT_CANVAS_PANEL_HH
#include "wx/panel.h"
#include "app/canvas.hh"
#include "geo/int-rect.hh"
#include "gui/canvas-change-event.hh"
#include "gui/canvas-panel-contexts.hh"
#include "gui/canvas-state.hh"
#include "gui/menu-predicate.hh"
#include "gui/mouse-capture.hh"
#include "tools/tool.hh"
#include "tools/tool-wrapper.hh"
#include "util/command-history.hh"
#include "util/distinct.hh"
#include "util/grid.hh"
#include "util/id-types.hh"
#include "util/image-list.hh"
#include "util-wx/file-path.hh"

class wxFileDropTarget;

namespace faint{

class ArtContainer;
class IntPoint;
class PosInfo;
class ToolModifiers;

enum class PreemptResult{ NONE, COMMIT, CANCEL };
enum class PreemptOption{ ALLOW_COMMAND, DISCARD_COMMAND };

using CanvasChangeTag = const wxEventTypeTag<CanvasChangeEvent>;
extern CanvasChangeTag EVT_FAINT_CANVAS_CHANGE;
extern CanvasChangeTag EVT_FAINT_GRID_CHANGE;
extern CanvasChangeTag EVT_FAINT_ZOOM_CHANGE;

class category_canvas_panel;
using initially_dirty = Distinct<bool, category_canvas_panel, 1>;

class Contexts{
public:
  Contexts(AppContext& app) :
    app(app),
    tool(app)
  {}

  const Tool& GetTool() const{
    return tool.GetTool();
  }

  Tool& GetTool(){
    return tool.GetTool();
  }

  AppContext& app;
  std::unique_ptr<TargetableCommandContext> command;
  std::unique_ptr<Canvas> canvas;
  ToolWrapper tool;
};

using convert_cursor_f = std::function<wxCursor(Cursor)>;

class CanvasPanel : public wxPanel {
public:
  CanvasPanel(wxWindow* parent,
    ImageList&&,
    const initially_dirty&,
    wxFileDropTarget*,
    const ArtContainer&,
    AppContext&,
    StatusInterface&);
  bool AcceptsFocus() const override;
  void AdjustHorizontalScrollbar(int pos);
  void AdjustScrollbars(const IntPoint&);
  void AdjustVerticalScrollbar(int pos);
  bool CanRedo() const;
  bool CanUndo() const;
  void CenterView(const IntPoint& ptView);
  void CenterViewImage(const Point& ptImage);
  void ChangeZoom(ZoomLevel::ChangeType);
  void ClearPointOverlay();
  void CloseUndoBundle(const utf8_string& name);
  void DeselectObject(Object*);
  void DeselectObjects();
  void DeselectObjects(const objects_t&);
  PosInfo ExternalHitTest(const IntPoint&);
  IntPoint GetFaintScrollPos() const;
  CanvasId GetCanvasId() const;
  Optional<FilePath> GetFilePath() const;
  Grid GetGrid() const;
  RasterSelection& GetImageSelection();
  const ImageList& GetImageList() const;
  Point GetImageViewStart() const;
  Canvas& GetInterface();
  int GetMaxScrollRight() const;
  int GetMaxScrollDown() const;
  IntPoint GetMaxUsefulScroll() const;
  MenuFlags GetMenuFlags() const;
  const objects_t& GetObjects();
  const objects_t& GetObjectSelection();
  Optional<IntPoint> GetPointOverlay() const;
  utf8_string GetRedoName() const;
  Point GetRelativeMousePos();
  Index GetSelectedFrame() const;
  utf8_string GetUndoName() const;
  coord GetZoom() const;
  const ZoomLevel& GetZoomLevel() const;
  bool Has(const ObjectId&);
  bool HasObjectSelection() const;
  bool HasRasterSelection() const;
  bool HasSelection() const;
  bool HasToolSelection() const;
  bool InSelection(const Point&);
  bool IsDirty() const;
  void MousePosRefresh();
  void NextFrame();
  void NotifySaved(const FilePath&);
  void OpenUndoBundle();
  PreemptResult Preempt(PreemptOption);
  void PreviousFrame();
  void Redo();
  void RunCommand(Command*);
  void RunCommand(Command*, const FrameId&);
  void RunDWIM();
  void ScrollMaxDown();
  void ScrollMaxLeft();
  void ScrollMaxRight();
  void ScrollMaxUp();
  void ScrollPageDown();
  void ScrollPageLeft();
  void ScrollPageRight();
  void ScrollPageUp();
  void SelectFrame(const Index&);
  void SelectObject(Object*, const deselect_old&);
  void SelectObjects(const objects_t&, const deselect_old&);
  void SetFaintScrollPos(const IntPoint&);
  void SetGrid(const Grid&);
  void SetMirage(const std::weak_ptr<Bitmap>&);
  void SetMirage(const std::weak_ptr<RasterSelection>&);
  void SetPointOverlay(const IntPoint&);
  void SetZoomLevel(const ZoomLevel&);
  void Undo();
  void ZoomFit();
private:
  friend class CanvasToolInterface;
  const RasterSelection& GetImageSelection() const;
  enum RefreshMode{REFRESH, NO_REFRESH};
  void CommitTool(Tool&, RefreshMode);
  int GetHorizontalPageSize() const;
  int GetVerticalPageSize() const;
  bool HandleToolResult(ToolResult);
  PosInfo HitTest(const IntPoint&, const ToolModifiers&);
  bool IgnoreCanvasHandle(const PosInfo&);
  void InclusiveRefresh(const IntRect&);
  void MousePosRefresh(const IntPoint&, const ToolModifiers&);
  void RefreshToolRect();
  void RunCommand(Command*, const clear_redo&, Image*);
  void ScrollLineUp();
  void ScrollLineDown();
  void ScrollLineLeft();
  void ScrollLineRight();
  void SendCanvasChangeEvent();
  void SendZoomChangeEvent();
  void SendGridChangeEvent();
  void SetFaintCursor(Cursor);
  void UndoObject(Command*);

  const ArtContainer& m_art;

  CanvasId m_canvasId;
  CommandHistory m_commands;
  Contexts m_contexts;

  struct {
    Optional<FilePath> filename;
    Optional<CommandId> savedAfter;
  } m_document;

  ImageList m_images;
  IntRect m_lastRefreshRect;

  struct {
    // Temporary replacements used during rendering instead of the
    // actual, content, to give direct feedback from e.g. dialogs, but
    // delaying modification of the actual image until a Command is
    // run.
    std::weak_ptr<Bitmap> bitmap; // Alternate background
    std::weak_ptr<RasterSelection> selection; // Alternate selection
  } m_mirage;

  MouseCapture m_mouse;

  bool m_probablyCtrlEnter = false; // \ref(ctrl-enter-workaround)

  struct {
    int startX = 0;
    int startY = 0;
    bool updateHorizontal = false;
    bool updateVertical = false;
  } m_scroll;

  CanvasState m_state;
  StatusInterface& m_statusInfo;
};

} // namespace

#endif
