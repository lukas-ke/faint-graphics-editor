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
#include "objects/object.hh" // mask_no_fill // Fixme
#include "rendering/faint-dc.hh"
#include "rendering/render-text.hh"
#include "text/slice.hh"
#include "text/split-string.hh"
#include "text/text-buffer.hh"
#include "text/text-expression-context.hh"
#include "text/text-expression.hh"
#include "util-wx/font.hh"
#include "util/default-settings.hh"
#include "util/iter.hh" // zip
#include "util/setting-id.hh"
#include "util/setting-util.hh"
#include "util/text-geo.hh" // text_selection_region

namespace faint{

utf8_string get_expression_string(const parse_result_t& result,
  const ExpressionContext& context)
{
  return result.Visit(
    [&](const ExpressionTree& tree){
      return tree.Evaluate(context).Visit(
        [](const utf8_string& result){
          return result;
        },
        [](const ExpressionEvalError& error){
          return error.description;
        });
    },
    [](const ExpressionParseError& error){
      return error.description;
    });
}

static utf8_string get_evaluated_string(const ExpressionContext& ctx,
  const Optional<parse_result_t>& maybeExpression,
  const TextBuffer& textBuf)
{
  return maybeExpression.Visit(
    [&](const parse_result_t& expression){
      return get_expression_string(expression, ctx);
    },
    [&](){
      return textBuf.get();
    });
}

static LineSegment compute_caret(const TextInfo& info,
  const TextBuffer& textBuf,
  const Tri& tri,
  const text_lines_t& lines,
  const Settings& settings)
{
  TextPos pos = caret_index_to_row_column(lines, textBuf.caret()); // Fixme: use caret_index_to_row_column

  const auto& line = lines[pos.row];
  auto textSize = info.TextSize(slice_up_to(line.text, pos.col));

  textSize.h = info.ComputeRowHeight();
  Tri caretTri(tri.P0(), tri.P1(), textSize.h);
  caretTri = aligned(offset_aligned(caretTri, textSize.w,
    static_cast<coord>(pos.row * textSize.h)),
    settings.Get(ts_HorizontalAlign), line.width, tri.Width());

  return LineSegment(caretTri.P0(), caretTri.P2());
}

TextRenderInfo render_text(
  FaintDC& dc,
  const TextBuffer& textBuf,
  const Optional<parse_result_t>& expression,
  const Tri& tri,
  bool currentlyEdited,
  TextInfo& textInfo,
  ExpressionContext& ctx,
  const Settings& settings)
{
  auto boundedText = settings.Get(ts_BoundedText);
  max_width_t maxWidth = boundedText ?
    max_width_t(tri.Width()) :
    max_width_t();

  text_lines_t lines = (currentlyEdited || !settings.Get(ts_ParseExpressions)) ?
    split_string(textInfo, textBuf.get(), maxWidth) :
    split_string(textInfo, get_evaluated_string(ctx, expression, textBuf),
      maxWidth);

  auto rowHeight = textInfo.ComputeRowHeight();

  if (textBuf.empty()){
    // Fixme: Tricky that this is done during rendering
    // Compute the caret position
    Tri caretTri(tri.P0(), tri.P1(), rowHeight);
    caretTri = offset_aligned(caretTri, 1.0, 0.0);

    return {rowHeight, LineSegment(caretTri.P0(), caretTri.P2())};
  }

  Align align(settings.Get(ts_HorizontalAlign),
    settings.Get(ts_VerticalAlign));

  // Fixme: Not every time
  Settings highlightSettings = default_rectangle_settings();
  highlightSettings.Set(ts_FillStyle, FillStyle::FILL);
  Color highlightColor = get_highlight_color();
  highlightSettings.Set(ts_Fg, Paint(highlightColor));
  highlightSettings.Set(ts_Bg, Paint(highlightColor));

  if (currentlyEdited){
    // Draw the selection highlighting
    for (const Tri& rowTri : text_selection_region(textInfo,
        tri,
        lines,
        textBuf.get_sel_range(),
        align))
    {
      dc.Rectangle(rowTri, highlightSettings);
    }
  }
  auto tris(text_line_regions(textInfo, tri, lines, align));
  Optional<Tri> clipTri(tri, boundedText);

  for (const auto item : zip(tris, lines)){
    // Fixme: Selected text should be drawn with wxSYS_COLOUR_HIGHLIGHTTEXT
    dc.Text(item.first, item.second.text, settings, clipTri);
  }

  return {rowHeight,
      compute_caret(textInfo, textBuf, tri, lines, settings)};
}

void render_text_mask(
  FaintDC& dc,
  const TextBuffer& textBuf,
  const Tri& tri,
  TextInfo& textInfo,
  const Settings& settings)
{
  // Fill the entire text region with "transparent inside" indicator.
  Settings s(default_rectangle_settings());
  s.Set(ts_FillStyle, FillStyle::BORDER_AND_FILL);
  s.Set(ts_Fg, Paint(mask_no_fill));
  s.Set(ts_Bg, Paint(mask_no_fill));
  dc.Rectangle(tri, s);

  // Fill the regions actually occupied by characters with "filled
  // inside" indicator.
  s.Set(ts_Fg, Paint(mask_fill));
  s.Set(ts_Bg, Paint(mask_fill));
  text_lines_t lines = split_string(textInfo, textBuf.get(),
    max_width_t(tri.Width()));

  const Align align(settings.Get(ts_HorizontalAlign),
    settings.Get(ts_VerticalAlign));

  // Draw a rectangle from the left edge to the right for each line of text
  for (const Tri& rowTri : text_selection_region(textInfo,
      tri, lines, textBuf.all(), align))
  {
    dc.Rectangle(rowTri, s);
  }
}

} // namespace
