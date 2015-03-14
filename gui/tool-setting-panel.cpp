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

#include "gui/art-container.hh"
#include "gui/drop-down-ctrl.hh"
#include "gui/font-ctrl.hh"
#include "gui/freezer.hh"
#include "gui/image-toggle-ctrl.hh"
#include "gui/spin-ctrl.hh"
#include "gui/tool-setting-ctrl.hh"
#include "gui/tool-setting-panel.hh"
#include "util/setting-id.hh"
#include "util/settings.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"

namespace faint{

class TestStringSource : public StringSource{
public:
  std::vector<utf8_string> Get() const override{
    return {"px", "mm", "cm", "m"}; // Fixme: Not here, and base on calibration
  }
};

TestStringSource g_testSrc;

ToolSettingPanel::ToolSettingPanel(wxWindow* parent,
  StatusInterface& status,
  ArtContainer& art,
  DialogContext& dialogContext) :
  wxPanel(parent)
{
  using namespace layout;

  const IntSize iconSize(28, 23);

  auto hline = [this](){
    return SizerItem(create_hline(this), Proportion(0), wxEXPAND|wxUP|wxDOWN, 5);
  };

  auto control = [this](ToolSettingCtrl* ctrl){
    // Add the control to the list of "managed controls", and return
    // a SizerItem so this can be used with create_column
    m_toolControls.push_back(ctrl);
    return SizerItem(ctrl, Proportion(0), wxALIGN_CENTER_HORIZONTAL|wxDOWN, 5);
  };

  auto bool_image_toggle =
    [&](const BoolSetting& s, Icon icon, const Tooltip& tooltip){
      return control(create_bool_image_toggle(this,
        s,
        art.Get(icon),
        status,
        tooltip));
    };

  auto int_spinner =
    [&](const IntSetting& s, const char* label){
    static const int initialValue = 1;
    return control(create_int_spinner(this, wxSize(50, -1), s, initialValue,
      label));
  };

  auto semi_float_spinner =
    [&](const FloatSetting& s, const char* label){
    static float initialValue = 1;
    return control(create_semi_float_spinner(this, wxSize(50, -1), s, initialValue,
        label));
  };

  auto image_toggle =
    [&](const IntSetting& setting,
      const Tooltip& tooltip,
      const IntSize& size,
      const std::vector<ToggleImage>& images)
    {
      return control(
        create_image_toggle(this,
          setting,
          size,
          status,
          tooltip,
          images));
    };

  auto img =
    [&](Icon icon, auto value, const char* statusText){
      return ToggleImage(art.Get(icon), to_int(value), statusText);
    };

  // Create all controls, and add them to m_toolControls.
  set_sizer(this, create_column(OuterSpacing(0), ItemSpacing(0), {
    hline(),

    bool_image_toggle(ts_EditPoints, Icon::EDIT_POINTS, Tooltip("Edit Points")),

    semi_float_spinner(ts_LineWidth, "Line width"),

    int_spinner(ts_BrushSize, "Brush Size"),

    control(create_font_ctrl(this, dialogContext)),

    // Since the font size spinner is right below the font button, "Size" is
    // sufficient as the label.
    int_spinner(ts_FontSize, "Size"),

    bool_image_toggle(ts_BoundedText, Icon::TEXT_BOUNDED,
      Tooltip("Bounded by rectangle")),

    bool_image_toggle(ts_ParseExpressions, Icon::TEXT_PARSING,
      Tooltip("Parse special commands")),

    bool_image_toggle(ts_ClosedPath, Icon::PATH_CLOSED,
      Tooltip("Closed path")),

    image_toggle(ts_HorizontalAlign, Tooltip("Text alignment"),
      iconSize,
      {img(Icon::TEXT_ALIGN_LEFT, HorizontalAlign::LEFT, "Left aligned text"),
       img(Icon::TEXT_ALIGN_CENTER, HorizontalAlign::CENTER, "Centered text"),
       img(Icon::TEXT_ALIGN_RIGHT, HorizontalAlign::RIGHT, "Right aligned text")}),

    image_toggle(ts_LineStyle, Tooltip("Line style"),
      IntSize(28,15),
      {img(Icon::LINESTYLE_SOLID, LineStyle::SOLID, "Solid Lines"),
       img(Icon::LINESTYLE_LONG_DASH, LineStyle::LONG_DASH, "Dashed lines")}),

    image_toggle(ts_PointType, Tooltip("Segment type"), iconSize,
      {img(Icon::POINTTYPE_LINE, PointType::LINE, "Line"),
       img(Icon::POINTTYPE_CURVE, PointType::CURVE, "Curve")}),

    image_toggle(ts_FillStyle, Tooltip("Fill style"), iconSize,
      {img(Icon::FILLSTYLE_BORDER, FillStyle::BORDER, "Border Only"),
       img(Icon::FILLSTYLE_FILL, FillStyle::FILL, "Fill Only"),
       img(Icon::FILLSTYLE_BORDER_AND_FILL, FillStyle::BORDER_AND_FILL,
         "Border and Fill")}),

    image_toggle(ts_BrushShape, Tooltip("Brush shape"), iconSize,
      {img(Icon::BRUSH_CIRCLE, BrushShape::CIRCLE, "Circular Brush"),
       img(Icon::BRUSH_RECT, BrushShape::SQUARE, "Square Brush"),
       img(Icon::BRUSH_EXPERIMENTAL, BrushShape::EXPERIMENTAL,
         "Experimental Brush")}),

    image_toggle(ts_LineArrowhead, Tooltip("Line end style"), iconSize,
      {img(Icon::LINE_NO_ARROW, LineArrowhead::NONE, "No arrowhead"),
       img(Icon::LINE_ARROW_FRONT, LineArrowhead::FRONT, "Forward arrowhead")}),

    image_toggle(ts_BackgroundStyle,
      Tooltip("Transparent or opaque background color"),
      iconSize,
      {img(Icon::CHOICE_TRANSPARENT, BackgroundStyle::MASKED,
          "Transparent Background"),
       img(Icon::CHOICE_OPAQUE, BackgroundStyle::SOLID, "Opaque Background")}),

    bool_image_toggle(ts_PolyLine, Icon::LINE_POLYLINE, Tooltip("Polyline")),

    bool_image_toggle(ts_AlphaBlending, Icon::ALPHA_BLENDING,
      Tooltip("Alpha blending")),

    control(create_drop_down(this,
      ts_Unit,
      g_testSrc,
      Tooltip("Unit"))),

    image_toggle(ts_TapeStyle, Tooltip("Tape measure"), iconSize,
     {img(Icon::TAPE_MEASURE_LINE, TapeMeasureStyle::LINE, "Line length"),
      img(Icon::TAPE_MEASURE_TRIANGLE, TapeMeasureStyle::TRIANGLE,
        "Triangle lengths")}),

    // Finishing separator
    hline()}));
}

void ToolSettingPanel::ShowSettings(const Settings& settings){
  auto freezer = freeze(this);
  for (auto ctrl : m_toolControls){
    bool show = ctrl->UpdateControl(settings);
    ctrl->Show(show);
  }
  Layout();
}

} // namespace
