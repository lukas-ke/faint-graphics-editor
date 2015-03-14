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

#include "bitmap/color.hh"
#include "util/default-settings.hh"
#include "util/setting-id.hh"
#include "util-wx/font.hh"

namespace faint{

// Setting init functions, for presumably faster retrieval
Settings init_default_rectangle_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_AntiAlias, true);
  s.Set(ts_Bg, Paint(Color(255,255,255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_FillStyle, FillStyle::BORDER);
  s.Set(ts_LineJoin, LineJoin::MITER);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1);
  s.Set(ts_SwapColors, false);
  s.Set(ts_Filter, 0);
  s.Set(ts_RadiusX, 0.0);
  s.Set(ts_RadiusY, 0.0);
  return s;
}

Settings init_settings_selection_rectangle(){
  Settings s(default_rectangle_settings());
  s.Set(ts_AntiAlias, false);
  s.Set(ts_Bg, Paint(Color(0, 0, 0)));
  s.Set(ts_Fg, Paint(Color(255, 0, 255)));
  s.Set(ts_FillStyle, FillStyle::BORDER);
  s.Set(ts_LineStyle, LineStyle::LONG_DASH);
  s.Set(ts_LineWidth, 1);
  return s;
}

Settings init_default_ellipse_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_AntiAlias, true);
  s.Set(ts_Bg, Paint(Color(255, 255, 255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_FillStyle, FillStyle::BORDER);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1);
  s.Set(ts_SwapColors, false);
  s.Set(ts_Filter, 0);
  return s;
}

Settings init_default_line_settings(){
  Settings s;
  s.Set(ts_AntiAlias, true);
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_LineArrowhead, LineArrowhead::NONE);
  s.Set(ts_LineCap, LineCap::ROUND);
  s.Set(ts_LineJoin, LineJoin::ROUND);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1);
  s.Set(ts_SwapColors, false);
  s.Set(ts_Filter, 0);
  return s;
}

Settings init_default_polygon_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_AntiAlias, true);
  s.Set(ts_Bg, Paint(Color(255, 255, 255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_FillStyle, FillStyle::BORDER);
  s.Set(ts_LineJoin, LineJoin::MITER);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1);
  s.Set(ts_SwapColors, false);
  return s;
}

Settings init_default_path_settings(){
  Settings s = default_polygon_settings();
  s.Set(ts_ClosedPath, true);
  return s;
}

Settings init_default_raster_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_BackgroundStyle, BackgroundStyle::SOLID);
  s.Set(ts_Bg, Paint(Color(255,255,255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  return s;
}

Settings init_default_spline_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_AntiAlias, true);
  s.Set(ts_Bg, Paint(Color(255, 255, 255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_LineCap, LineCap::BUTT);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1);
  s.Set(ts_SwapColors, false);
  return s;
}

Settings init_default_text_settings(){
  Settings s;
  s.Set(ts_AlignedResize, false);
  s.Set(ts_Bg, Paint(Color(255, 255, 255)));
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_FontBold, false);
  s.Set(ts_FontFace, get_default_font_name());
  s.Set(ts_FontItalic, false);
  s.Set(ts_FontSize, get_default_font_size());
  s.Set(ts_SwapColors, false);
  s.Set(ts_BoundedText, true);
  s.Set(ts_HorizontalAlign, HorizontalAlign::LEFT);
  s.Set(ts_VerticalAlign, VerticalAlign::TOP);
  s.Set(ts_ParseExpressions, false);
  s.Set(ts_TextRenderStyle, TextRenderStyle::CAIRO_PATH);
  return s;
}

Settings init_default_tool_settings(){
  Settings s;
  s.Set(ts_AlphaBlending, false);
  s.Set(ts_BackgroundStyle, BackgroundStyle::MASKED);
  s.Set(ts_Bg, Paint(Color(255,255,255)));
  s.Set(ts_BrushShape, BrushShape::SQUARE);
  s.Set(ts_BrushSize, 5);
  s.Set(ts_Fg, Paint(Color(0,0,0)));
  s.Set(ts_FillStyle, FillStyle::BORDER);
  s.Set(ts_Filter, 0);
  s.Set(ts_FontBold, false);
  s.Set(ts_FontFace, get_default_font_name());
  s.Set(ts_FontItalic, false);
  s.Set(ts_FontSize, get_default_font_size());
  s.Set(ts_LineArrowhead, LineArrowhead::NONE);
  s.Set(ts_LineCap, LineCap::BUTT);
  s.Set(ts_LineStyle, LineStyle::SOLID);
  s.Set(ts_LineWidth, 1.0);
  s.Set(ts_PointType, PointType::LINE);
  s.Set(ts_PolyLine, false);
  s.Set(ts_SwapColors, false);
  s.Set(ts_TapeStyle, TapeMeasureStyle::LINE);
  s.Set(ts_Unit, "px");
  return s;
}

const Settings& default_ellipse_settings(){
  static Settings s(init_default_ellipse_settings());
  return s;
}

const Settings& default_line_settings(){
  static Settings s(init_default_line_settings());
  return s;
}

const Settings& default_path_settings(){
  static Settings s(init_default_path_settings());
  return s;
}

const Settings& default_polygon_settings(){
  static Settings s(init_default_polygon_settings());
  return s;
}

const Settings& default_raster_settings(){
  static Settings s(init_default_raster_settings());
  return s;
}

const Settings& default_rectangle_settings(){
  static Settings s(init_default_rectangle_settings());
  return s;
}

const Settings& default_spline_settings(){
  static Settings s(init_default_spline_settings());
  return s;
}

const Settings& default_text_settings(){
  static Settings s(init_default_text_settings());
  return s;
}

const Settings& default_tool_settings(){
  static Settings s(init_default_tool_settings());
  return s;
}

Settings default_bitmap_settings(){
  Settings s;
  s.Set(ts_AlphaBlending, false);
  s.Set(ts_BackgroundStyle, BackgroundStyle::SOLID);
  s.Set(ts_Bg, Paint(Color(0,0,0)));
  return s;
}

const Settings& selection_rectangle_settings(){
  static Settings s(init_settings_selection_rectangle());
  return s;
}

} // namespace
