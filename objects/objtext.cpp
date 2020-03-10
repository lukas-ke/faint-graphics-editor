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

#include <cassert>
#include "bitmap/bitmap.hh"
#include "commands/tri-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/int-size.hh"
#include "geo/pathpt.hh"
#include "geo/pixel-snap.hh"
#include "geo/size.hh"
#include "objects/objtext.hh"
#include "rendering/faint-dc.hh"
#include "rendering/render-text.hh"
#include "rendering/text-info-dc.hh"
#include "text/slice.hh"
#include "text/split-string.hh"
#include "text/text-geo.hh"
#include "text/text-expression.hh"
#include "text/utf8-string.hh"
#include "util-wx/font.hh"
#include "util/default-settings.hh"
#include "util/iter.hh"
#include "util/optional.hh"
#include "util/settings.hh"
#include "util/type-util.hh"

namespace faint{

static utf8_string get_expression_string(const parse_result_t& result,
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

static LineSegment empty_text_caret(const Tri& tri, coord rowHeight){
  Tri caretTri(tri.P0(), tri.P1(), rowHeight);
  caretTri = offset_aligned(caretTri, 1.0, 0.0);
  return LineSegment(caretTri.P0(), caretTri.P2());

}

ObjText::ObjText(const Tri& tri, const utf8_string& text, const Settings& s)
  : StandardObject(s),
    m_textBuf(text),
    m_caret(LineSegment(Point(0,0),Point(0,0))),
    m_tri(tri)
{
  Init();
}

ObjText::ObjText(const ObjText& other)
  : StandardObject(other.GetSettings()),
    m_textBuf(other.m_textBuf),
    m_caret(LineSegment(Point(0,0),Point(0,0))),
    m_tri(other.GetTri()),
    m_expression(other.m_expression)
{
  Init();
}

void ObjText::Init(){
  assert(m_settings.Has(ts_FontSize));
  assert(m_settings.Has(ts_FontFace));
  m_beingEdited = false;

  m_rowHeight = TextInfoDC(m_settings).ComputeRowHeight();
  m_lastFontSize = m_settings.Get(ts_FontSize);
  m_lastFontFace = m_settings.Get(ts_FontFace);
  m_expression.Set(parse_text_expression(m_textBuf.get()));
}

coord ObjText::BaselineOffset() const{
  return TextInfoDC(m_settings).Ascent();
}

size_t ObjText::CaretPos(const Point& imagePos) const{
  TextInfoDC info(m_settings);
  const auto lines = split_string(info, m_textBuf.get(),
    max_width_t(m_tri.Width()));

  const auto maxCaret = m_textBuf.size();
  const auto rowHeight = RowHeight();

  return caret_index_from_pos(imagePos,
    m_tri,
    lines,
    rowHeight,
    maxCaret,
    [&](const utf8_string& s){return info.CumulativeTextWidth(s);});
}

Object* ObjText::Clone() const{
  return new ObjText(*this);
}

void ObjText::Draw(FaintDC& dc, ExpressionContext& ctx){
  TextInfoDC textInfo(m_settings);
  m_rowHeight = textInfo.ComputeRowHeight();

  if (m_textBuf.empty()){
    // Fixme: Tricky that this is done during rendering
    m_caret = empty_text_caret(m_tri, m_rowHeight);
  }

  auto lines = Split(textInfo, ctx);
  render_text(dc,
    lines,
    m_textBuf.get_sel_range(),
    m_tri,
    m_beingEdited,
    textInfo,
    m_settings);

  m_caret = compute_caret(textInfo,
    m_textBuf.caret(),
    m_tri,
    lines,
    m_settings);
}

void ObjText::DrawMask(FaintDC& dc, ExpressionContext& ctx){
  TextInfoDC textInfo(m_settings);
  render_text_mask(dc,
    Split(textInfo, ctx),
    m_tri,
    textInfo,
    m_settings,
    Paint(mask_no_fill),
    Paint(mask_fill));
}

coord ObjText::GetArea() const{
  return 0.0; // Fixme: Consider if ink area or full rectangle more useful than 0
}

Rect ObjText::GetAutoSizedRect() const {
  TextInfoDC info(m_settings);
  // Fixme: Doesn't take expressions in account.
  const auto textSize = text_extents(info,
    split_string(info, m_textBuf.get(), no_option()));

  return Rect(m_tri.P0(), floated(textSize));
}

LineSegment ObjText::GetCaret() const{
  return m_caret;
}

utf8_string ObjText::GetEvaluatedString(const ExpressionContext& ctx) const{
  if (m_settings.Get(ts_ParseExpressions) && m_expression.IsSet()){
    return get_expression_string(m_expression.Get(), ctx);
  }
  else{
    return m_textBuf.get();
  }
}

std::vector<PathPt> ObjText::GetPath(const ExpressionContext& ctx) const{
  TextInfoDC textInfo(m_settings);
  text_lines_t lines = split_string(textInfo, GetEvaluatedString(ctx),
    m_settings.Get(ts_BoundedText) ? max_width_t(m_tri.Width()) : no_option());

  Align align(m_settings.Get(ts_HorizontalAlign),
    m_settings.Get(ts_VerticalAlign));

  // Build the path
  std::vector<PathPt> path;
  auto tris(text_line_regions(textInfo, m_tri, lines, align));
  for (const auto& [tri, line] : zip(tris, lines)){
    const auto subPath(textInfo.GetTextPath(tri, line.text));
    for (const auto& p : subPath){
      path.push_back(p);
    }
  }
  return path;
}

utf8_string ObjText::GetRawString() const{
  return m_textBuf.get();
}

IntRect ObjText::GetRefreshRect() const{
  if (m_settings.Get(ts_BoundedText)){
    return floiled(bounding_rect(m_tri));
  }
  else{
    // Fixme: Slow.
    TextInfoDC info(m_settings);
    text_lines_t lines = split_string(info, m_textBuf.get(), max_width_t());

    int w = 0;
    for (auto l : lines){
      w = std::max(info.GetWidth(l.text), w);
    }

    const int h = info.ComputeRowHeight() * resigned(lines.size());
    return floiled(bounding_rect(Tri(m_tri.P0(), m_tri.GetAngle(), Size(w,h))));
  }
}

TextBuffer& ObjText::GetTextBuffer(){
  return m_textBuf;
}

Tri ObjText::GetTri() const{
  if (m_settings.Get(ts_BoundedText)){
    return m_tri;
  }
  else{
    TextInfoDC info(m_settings);
    const auto textSize = text_extents(info,
      split_string(info, m_textBuf.get(), no_option()));

    return Tri(m_tri.P0(), m_tri.GetAngle(), floated(textSize));
  }
}

utf8_string ObjText::GetType() const{
  return "Text Region";
}

bool ObjText::HasSelectedRange() const{
  return !m_textBuf.get_sel_range().Empty();
}

Optional<CmdFuncs> ObjText::PixelSnapFunc(){
  if (m_tri.Skew() != 0 || !multiple_of_90(m_tri.GetAngle())){
    return {};
  }

  return {CmdFuncs(
     [this](){
       m_tri = pixel_snap_middle(m_tri);
     },
     [oldTri = m_tri, this](){
       m_tri = oldTri;
     })};
}

coord ObjText::RowHeight() const{
  if (m_settings.Get(ts_FontSize) != m_lastFontSize ||
    m_lastFontFace != m_settings.Get(ts_FontFace))
  {
    m_lastFontSize = m_settings.Get(ts_FontSize);
    m_lastFontFace = m_settings.Get(ts_FontFace);
    m_rowHeight = TextInfoDC(m_settings).ComputeRowHeight();
  }
  return m_rowHeight;
}

void ObjText::SetCaretPos(size_t pos, bool select){
  m_textBuf.caret(pos, select);
}

void ObjText::SetEdited(bool edited){
  m_beingEdited = edited;
  if (m_beingEdited == false){
    m_expression.Set(parse_text_expression(m_textBuf.get()));
  }
}

void ObjText::SetTri(const Tri& t){
  m_tri = t;
}

bool ObjText::ShowSizeBox() const{
  // Since the text object boundaries aren't clear (like say a
  // rectangle), show a box while resizing.
  return true;
}

text_lines_t ObjText::Split(const TextInfo& textInfo,
  ExpressionContext& ctx) const
{
  const auto maxWidth = m_settings.Get(ts_BoundedText) ?
    max_width_t(m_tri.Width()) :
    max_width_t();

  return (m_beingEdited || !m_settings.Get(ts_ParseExpressions)) ?
    split_string(textInfo, m_textBuf.get(), maxWidth) :
    split_string(textInfo, get_evaluated_string(ctx, m_expression, m_textBuf),
      maxWidth);
}

text_lines_t split_evaluated(ExpressionContext& ctx, const ObjText& text){
  return split_string(TextInfoDC(text.GetSettings()),
    text.GetEvaluatedString(ctx),
    text.GetTri().Width());
}

CommandPtr crop_text_region_command(ObjText& text){
  const Tri oldTri = text.GetTri();
  Tri tri(rotated(tri_from_rect(text.GetAutoSizedRect()),
      oldTri.GetAngle(), oldTri.P0()));
  return tri_command(&text, New(tri), Old(oldTri), "Auto-Size");
}

bool is_text(const Object& obj){
  return is_type<ObjText>(obj);
}

ObjText* create_text_object_raw(const Tri& tri,
  const utf8_string& text,
  const Settings& settings)
{
  return new ObjText(tri, text, settings);
}

ObjectPtr create_text_object(const Tri& tri,
  const utf8_string& text,
  const Settings& settings)
{
  return std::make_unique<ObjText>(tri, text, settings);
}

} // namespace
