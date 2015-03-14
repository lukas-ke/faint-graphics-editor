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

#include <vector>
#include "wx/sizer.h"
#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/draw.hh"
#include "geo/int-size.hh"
#include "gui/art-container.hh"
#include "gui/events.hh"  // LayerChangeEvent
#include "gui/image-toggle-ctrl.hh"
#include "gui/setting-events.hh"
#include "gui/status-button.hh"
#include "gui/tool-bar.hh"
#include "gui/tool-drop-down-button.hh"
#include "util-wx/bind-event.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"
#include "util/color-bitmap-util.hh" // with_border
#include "util/setting-id.hh"

namespace faint{

const wxSize g_toolButtonSize(25,25);

const IntSetting g_layerSetting;

// Create the shaded "selected" tool graphic
static wxBitmap create_selected_bitmap(const wxBitmap& image){
  auto bg(with_border(Bitmap(to_faint(image.GetSize()), Color(113,152,207,209)),
      Color(113,153,208,115), LineStyle::SOLID));
  blend(at_top_left(to_faint(image)), onto(bg));
  return to_wx_bmp(bg);
}

class Toolbar::ToolbarImpl{
public:
  ToolbarImpl(wxWindow* parent,
    StatusInterface& status,
    ArtContainer& art)
    : m_activeButton(nullptr),
      m_groupButton(nullptr),
      m_panel(create_panel(parent))
  {
    using namespace layout;
    auto hline = [this](int down){
      return SizerItem(create_hline(m_panel),
        Proportion(0), wxEXPAND|wxDOWN, down);
    };

    m_layerChoice = create_image_toggle(m_panel, g_layerSetting,
      IntSize(30, 32),
      status,
      Tooltip("Raster or Object layer"),
      {ToggleImage(art.Get(Icon::LAYER_RASTER), to_int(Layer::RASTER),
       "Select the Raster Layer"),
       ToggleImage(art.Get(Icon::LAYER_OBJECT), to_int(Layer::OBJECT),
       "Select the Object Layer")},
       Axis::HORIZONTAL,
       IntSize(3, 3));

    set_sizer(m_panel, create_column(OuterSpacing(0), ItemSpacing(0), {
       {m_layerChoice, Proportion(0), wxEXPAND|wxDOWN, 5},
       hline(5),
       {CreateToolButtons(art, status),
           Proportion(0), wxALIGN_CENTER_HORIZONTAL}}));
    
    bind_fwd(m_panel, wxEVT_TOGGLEBUTTON,
      [&](wxCommandEvent& event){
        if (event.GetEventObject() != m_groupButton){
          SendToolChoiceEvent(to_tool_id(event.GetId()));
        }
        event.Skip();
      });

    bind_fwd(m_panel, EVT_FAINT_INT_SETTING_CHANGE,
      [&](const SettingEvent<IntSetting>& e){
        int layer = e.GetValue();
        LayerChangeEvent newEvent(to_layerstyle(layer));
        m_panel->GetEventHandler()->ProcessEvent(newEvent);
      });
  }

  wxWindow* AsWindow(){
    return m_panel;
  }

  void SendToolChoiceEvent(ToolId id){
    // Remove the selected indication from the current tool button
    m_activeButton->SetValue(false);

    auto it = m_idToButton.find(id);
    if (it != end(m_idToButton)){
      wxBitmapToggleButton* button = it->second;
      if (!button->IsEnabled()){
        // Do not select a disabled tool
        return;
      }

      // Add the selected indication to the new tool
      m_activeButton = button;
      m_activeButton->SetValue(true);
    }
    else if (m_groupButton->SetSelectedTool(id)){
      m_activeButton = m_groupButton;
    }
    else {
      // Tool without button.
    }

    ToolChangeEvent evt(id);
    m_panel->GetEventHandler()->ProcessEvent(evt);
  }

  void SendLayerChoiceEvent(Layer layer){
    m_layerChoice->SetValue(to_int(layer));
    m_layerChoice->SendChangeEvent();
  }

private:
  void AddTool(const wxBitmap& bmpInactive,
    const Tooltip& tooltip,
    const Description& description,
    ToolId id,
    wxGridSizer* sizer,
    StatusInterface& status)
  {
    wxBitmap bmpActive(create_selected_bitmap(bmpInactive));
    ToggleStatusButton* button = new ToggleStatusButton(m_panel,
      to_int(id),
      g_toolButtonSize,
      status,
      bmpInactive,
      bmpActive,
      tooltip,
      description);

    m_idToButton[id] = button;
    sizer->Add(button);

    if (m_activeButton == nullptr){
      m_activeButton = button;
    }
  }

  ToolDropDownButton* AddToolGroup(wxGridSizer* sizer,
    const std::vector<ToolInfo>& tools)
  {
    auto* button = tool_drop_down_button(m_panel, to_faint(g_toolButtonSize), tools);
    sizer->Add(button);
    bind_fwd(button, EVT_FAINT_TOOL_CHANGE,
      [&](ToolChangeEvent& event){
        SendToolChoiceEvent(event.GetTool());
      });
    return button;
  }

  wxSizer* CreateToolButtons(ArtContainer& art, StatusInterface& status){
    auto sizer = new wxGridSizer(0, 2, 0, 0);

    auto add_tool =
      [&](Icon icon,
        const Tooltip& tooltip,
        const Description& description,
        ToolId id)
      {
        AddTool(art.Get(icon),
          tooltip,
          description,
          id,
          sizer,
          status);
      };

    add_tool(Icon::TOOL_SELECTION,
      Tooltip("Selection"),
      Description("Adjusts the selection."),
      ToolId::SELECTION);

    add_tool(Icon::TOOL_PEN,
      Tooltip("Pen"),
      Description("Draws single pixels."),
      ToolId::PEN);

    add_tool(Icon::TOOL_BRUSH,
      Tooltip("Brush"),
      Description("Free hand drawing with selected brush shape and size."),
      ToolId::BRUSH);

    add_tool(Icon::TOOL_PICKER,
      Tooltip("Color picker"),
      Description("Selects a color in the image."),
      ToolId::PICKER);

    add_tool(Icon::TOOL_LINE,
      Tooltip("Line"),
      Description("Draws lines."),
      ToolId::LINE);

    add_tool(Icon::TOOL_PATH,
      Tooltip("Path"),
      Description("Draws paths."),
      ToolId::PATH);

    add_tool(Icon::TOOL_RECTANGLE,
      Tooltip("Rectangle"),
      Description("Draws rectangles or squares."),
      ToolId::RECTANGLE);

    add_tool(Icon::TOOL_ELLIPSE,
      Tooltip("Ellipse"),
      Description("Draws ellipses or circles."),
      ToolId::ELLIPSE);

    add_tool(Icon::TOOL_POLYGON,
      Tooltip("Polygon"),
      Description("Draws polygons."),
      ToolId::POLYGON);

    add_tool(Icon::TOOL_TEXT,
      Tooltip("Text"),
      Description("Writes text."),
      ToolId::TEXT);

    m_groupButton = AddToolGroup(sizer, {
      { art.Get(Icon::TOOL_CALIBRATE),
        create_selected_bitmap(art.Get(Icon::TOOL_CALIBRATE)),
        Tooltip("Calibrate"),
        Description("Defines image measurements."),
        ToolId::CALIBRATE},

      { art.Get(Icon::TOOL_LEVEL),
        create_selected_bitmap(art.Get(Icon::TOOL_LEVEL)),
        Tooltip("Level"),
        Description("Image alignment"),
        ToolId::LEVEL},

      { art.Get(Icon::TOOL_TAPE_MEASURE),
        create_selected_bitmap(art.Get(Icon::TOOL_TAPE_MEASURE)),
        Tooltip("Tape measure"),
        Description("Image measurements"),
        ToolId::TAPE_MEASURE},

      { art.Get(Icon::TOOL_HOT_SPOT),
        create_selected_bitmap(art.Get(Icon::TOOL_HOT_SPOT)),
        Tooltip("Hot spot"),
        Description("Sets the image hot spot."),
        ToolId::HOT_SPOT}});

    add_tool(Icon::TOOL_FLOODFILL,
      Tooltip("Fill"),
      Description("Fills contiguous areas."),
      ToolId::FLOOD_FILL);

    return sizer;
  }

private:
  wxBitmapToggleButton* m_activeButton;
  ToolDropDownButton* m_groupButton;
  wxWindow* m_panel;
  std::map<ToolId, wxBitmapToggleButton*> m_idToButton;
  IntSettingCtrl* m_layerChoice;
};

Toolbar::Toolbar(wxWindow* parent, StatusInterface& status, ArtContainer& art){
  m_impl = make_dumb<ToolbarImpl>(parent, status, art);
}

wxWindow* Toolbar::AsWindow(){
  return m_impl->AsWindow();
}

void Toolbar::SendLayerChoiceEvent(Layer layer){
  m_impl->SendLayerChoiceEvent(layer);
}

void Toolbar::SendToolChoiceEvent(ToolId toolId){
  m_impl->SendToolChoiceEvent(toolId);
}

} // namespace
