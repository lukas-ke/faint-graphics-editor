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

#include "commands/command.hh"
#include "tools/tool.hh"
#include "util/optional.hh"
#include "util/raster-selection.hh"
#include "util/setting-util.hh"

namespace faint{

Tool* raster_selection_tool(const Settings&, const ActiveCanvas&, ToolActions&);
Tool* object_selection_tool(const ActiveCanvas&, ToolActions&);

class SelectionTool : public Tool{
public:
  SelectionTool(Layer layer,
    const Settings& allSettings,
    const ActiveCanvas& activeCanvas,
    ToolActions& actions)
    : Tool(ToolId::SELECTION),
      m_activeTool(nullptr),
      m_objectSelectionTool(object_selection_tool(activeCanvas, actions)),
      m_rasterSelectionTool(raster_selection_tool(allSettings, activeCanvas,
        actions))
  {
    m_activeTool = layer == Layer::RASTER ?
      m_rasterSelectionTool.get() :
      m_objectSelectionTool.get();
  }

  ToolResult Char(const KeyInfo& key) override{
    return m_activeTool->Char(key);
  }

  ToolResult DoubleClick(const PosInfo& info) override{
    return m_activeTool->DoubleClick(info);
  }

  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override{
    m_activeTool->Draw(dc, overlays, info);
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return m_activeTool->DrawBeforeZoom(layer);
  }

  bool EatsSettings() const override{
    return m_activeTool->EatsSettings();
  }

  CommandPtr GetCommand() override{
    return m_activeTool->GetCommand();
  }

  Cursor GetCursor(const PosInfo& pos) const override{
    return m_activeTool->GetCursor(pos);
  }

  IntRect GetRefreshRect(const RefreshInfo& info) const override{
    return m_activeTool->GetRefreshRect(info);
  }

  const Settings& GetSettings() const override{
    return m_activeTool->GetSettings();
  }

  Optional<const faint::HistoryContext&> HistoryContext() const override{
    return const_cast<const Tool*>(m_activeTool)->HistoryContext();
  }

  ToolResult MouseDown(const PosInfo& info) override{
    return m_activeTool->MouseDown(info);
  }

  ToolResult MouseUp(const PosInfo& info) override{
    return m_activeTool->MouseUp(info);
  }

  ToolResult MouseMove(const PosInfo& info) override{
    return m_activeTool->MouseMove(info);
  }

  ToolResult Preempt(const PosInfo& info) override{
    // Preempting only the active selection tool (i.e. either raster
    // or object) seems to be enough.
    return m_activeTool->Preempt(info);
  }

  bool RefreshOnMouseOut() const override{
    return m_activeTool->RefreshOnMouseOut();
  }

  void SelectionChange() override{
    m_activeTool->SelectionChange();
  }

  Optional<const faint::SelectionContext&> SelectionContext() const override{
    return const_cast<const Tool*>(m_activeTool)->SelectionContext();
  }

  bool Set(const BoundSetting& s) override{
    return m_activeTool->Set(s);
  }

  void SetLayer(Layer layer) override{
    if (layer == Layer::RASTER){
      m_activeTool = m_rasterSelectionTool.get();
    }
    else if (layer == Layer::OBJECT){
      m_activeTool = m_objectSelectionTool.get();
    }
  }

  Optional<const faint::TextContext&> TextContext() const override{
    return const_cast<const Tool*>(m_activeTool)->TextContext();
  }

  bool UpdateSettings(const Settings& s) override{
    return m_activeTool->UpdateSettings(s);
  }

private:
  Tool* m_activeTool;
  std::unique_ptr<Tool> m_objectSelectionTool;
  std::unique_ptr<Tool> m_rasterSelectionTool;
};

Tool* selection_tool(Layer layer,
  const Settings& allSettings,
  const ActiveCanvas& activeCanvas,
  ToolActions& actions)
{
  return new SelectionTool(layer, allSettings, activeCanvas, actions);
}

} // namespace
