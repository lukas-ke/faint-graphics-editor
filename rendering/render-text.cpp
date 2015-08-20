// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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
#include "geo/tri.hh"
#include "rendering/faint-dc.hh"
#include "rendering/render-text.hh"
#include "text/slice.hh"
#include "text/split-string.hh"
#include "text/text-buffer.hh"
#include "util-wx/font.hh"
#include "util/default-settings.hh"
#include "util/iter.hh" // zip
#include "util/setting-id.hh"
#include "util/setting-util.hh"
#include "util/text-geo.hh" // text_selection_region

namespace faint{

LineSegment compute_caret(const TextInfo& info,
  const TextBuffer& textBuf,
  const Tri& tri,
  const text_lines_t& lines,
  const Settings& settings)
{
  TextPos pos = caret_index_to_row_column(lines, textBuf.caret());

  const auto& line = lines[pos.row];
  auto textSize = info.TextSize(slice_up_to(line.text, pos.col));

  textSize.h = info.ComputeRowHeight();
  Tri caretTri(tri.P0(), tri.P1(), textSize.h);
  caretTri = aligned(offset_aligned(caretTri, textSize.w,
    static_cast<coord>(pos.row * textSize.h)),
    settings.Get(ts_HorizontalAlign), line.width, tri.Width());

  return LineSegment(caretTri.P0(), caretTri.P2());
}

extern const Color mask_no_fill;
extern const Color mask_fill;

static auto get_text_highlight_settings(){
  // Fixme: Not every time
  Settings highlightSettings = default_rectangle_settings();
  highlightSettings.Set(ts_FillStyle, FillStyle::FILL);
  Color highlightColor = get_highlight_color();
  highlightSettings.Set(ts_Fg, Paint(highlightColor));
  highlightSettings.Set(ts_Bg, Paint(highlightColor));
  return highlightSettings;
}

static void draw_selection_highlighting(FaintDC& dc,
  TextInfo& textInfo,
  const Tri& tri,
  const text_lines_t& lines,
  const CaretRange& selectionRange,
  const Align& align)
{
  // Draw the selection highlighting
  auto highlightSettings = get_text_highlight_settings();
  auto regions = text_selection_region(textInfo,
    tri,
    lines,
    selectionRange,
    align);
  for (const Tri& r : regions){
    dc.Rectangle(r, highlightSettings);
  }
}

static void draw_text(FaintDC& dc,
  const text_lines_t& lines,
  const TextInfo& textInfo,
  const Tri& tri,
  const Align& align,
  const Settings& settings)
{
  auto tris(text_line_regions(textInfo, tri, lines, align));
  Optional<Tri> clipTri(tri, settings.Get(ts_BoundedText));

  for (const auto item : zip(tris, lines)){
    // Fixme: Selected text should be drawn with wxSYS_COLOUR_HIGHLIGHTTEXT
    dc.Text(item.first, item.second.text, settings, clipTri);
  }
}

void render_text(FaintDC& dc,
  const text_lines_t& lines,
  const CaretRange& selectionRange,
  const Tri& tri,
  bool currentlyEdited,
  TextInfo& textInfo,
  const Settings& settings)
{
  Align align(settings.Get(ts_HorizontalAlign), settings.Get(ts_VerticalAlign));

  if (currentlyEdited){
    draw_selection_highlighting(dc, textInfo, tri, lines, selectionRange, align);
  }

  draw_text(dc, lines, textInfo, tri, align, settings);
}

static auto rect_mask(const Paint& p){
  Settings s(default_rectangle_settings());
  s.Set(ts_FillStyle, FillStyle::BORDER_AND_FILL);
  s.Set(ts_Fg, p);
  s.Set(ts_Bg, p);
  return s;
}

void render_text_mask(FaintDC& dc,
  const text_lines_t& lines,
  const Tri& tri,
  TextInfo& textInfo,
  const Settings& settings,
  const Paint& withinBounds,
  const Paint& withinLine)
{
  // Fill the entire text region with "transparent inside" indicator.
  dc.Rectangle(tri, rect_mask(withinBounds));

  const Align align(settings.Get(ts_HorizontalAlign),
    settings.Get(ts_VerticalAlign));

  // Draw a rectangle from the left edge to the right for each line of text
  const auto filledMask = rect_mask(withinLine);
  const auto regions = text_line_regions(textInfo, tri, lines, align);
  for (const Tri& r : regions){
    dc.Rectangle(r, filledMask);
  }
}

} // namespace
