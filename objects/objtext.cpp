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
#include "text/slice.hh"
#include "text/split-string.hh"
#include "text/text-expression.hh"
#include "text/utf8-string.hh"
#include "util-wx/font.hh"
#include "util/default-settings.hh"
#include "util/iter.hh"
#include "util/optional.hh"
#include "util/settings.hh"
#include "util/text-geo.hh"
#include "rendering/render-text.hh"

namespace faint{

class TextInfoDC : public TextInfo{
public:
  explicit TextInfoDC(const Settings& settings)
    : m_bitmap(IntSize(10,10)),
      m_dc(m_bitmap),
      m_settings(settings)
  {}

  int GetWidth(const utf8_string& str) const override{
    return m_dc.TextSize(str, m_settings).w;
  }

  IntSize TextSize(const utf8_string& str) const override{
    return m_dc.TextSize(str, m_settings);
  }

  int ComputeRowHeight() const override{
    // A full row is the ascent + descent
    if (m_rowHeight.NotSet()){
      auto metrics = m_dc.GetFontMetrics(m_settings);
      m_rowHeight.Set(metrics.ascent + metrics.descent);
    }
    return m_rowHeight.Get();
  }

  coord Ascent() const{
    return floated(m_dc.GetFontMetrics(m_settings).ascent);
  }

  std::vector<int> CumulativeTextWidth(const utf8_string& str){
    return m_dc.CumulativeTextWidth(str, m_settings);
  }

  std::vector<PathPt> GetTextPath(const Tri& tri, const utf8_string& str){
    return m_dc.GetTextPath(tri, str, m_settings);
  }

  TextMeasures TextExtents(const utf8_string& str){
    return m_dc.TextExtents(str, m_settings);
  }

  FontMetrics GetFontMetrics(){
    return m_dc.GetFontMetrics(m_settings);
  }

  TextInfoDC& operator=(const TextInfoDC&) = delete;
  TextInfoDC(const TextInfoDC&) = delete;
private:
  Bitmap m_bitmap;
  FaintDC m_dc;
  const Settings& m_settings;
  mutable Optional<int> m_rowHeight;
};

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

ObjText::ObjText(const Tri& tri, const utf8_string& text, const Settings& s)
  : Object(s),
    m_textBuf(text),
    m_caret(LineSegment(Point(0,0),Point(0,0))),
    m_tri(tri)
{
  Init();
}

ObjText::ObjText(const ObjText& other)
  : Object(other.GetSettings()),
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

  m_highlightSettings = default_rectangle_settings();
  m_highlightSettings.Set(ts_FillStyle, FillStyle::FILL);
  Color highlightColor = get_highlight_color();
  m_highlightSettings.Set(ts_Fg, Paint(highlightColor));
  m_highlightSettings.Set(ts_Bg, Paint(highlightColor));

  m_rowHeight = TextInfoDC(m_settings).ComputeRowHeight();
  m_lastFontSize = m_settings.Get(ts_FontSize);
  m_lastFontFace = m_settings.Get(ts_FontFace);
  m_expression.Set(parse_text_expression(m_textBuf.get()));
}

LineSegment ObjText::ComputeCaret(const TextInfo& info, const Tri& tri,
  const text_lines_t& lines)
{
  TextPos pos = index_to_row_column(lines, m_textBuf.caret());

  const auto& line = lines[pos.row];
  auto textSize = info.TextSize(slice_up_to(line.text, pos.col));

  textSize.h = info.ComputeRowHeight();
  Tri caretTri(tri.P0(), tri.P1(), textSize.h);
  caretTri = aligned(offset_aligned(caretTri, textSize.w,
    static_cast<coord>(pos.row * textSize.h)),
    m_settings.Get(ts_HorizontalAlign), line.width, tri.Width());

  return LineSegment(caretTri.P0(), caretTri.P2());
}

coord ObjText::BaselineOffset() const{
  return TextInfoDC(m_settings).Ascent();
}

size_t ObjText::CaretPos(const Point& posUnaligned) const{
  Tri tri = m_tri;

  // Realign the tri with the image to simplify calculations
  Point pos(rotate_point(posUnaligned, -tri.GetAngle(), tri.P0()));
  tri = rotated(tri, -tri.GetAngle(), tri.P0());

  if (pos.y < tri.P0().y){
    return 0;
  }

  TextInfoDC info(m_settings);

  text_lines_t lines = split_string(info, m_textBuf.get(),
    max_width_t(tri.Width()));
  const coord rowHeight = RowHeight();
  const size_t row = static_cast<size_t>((pos.y - tri.P0().y) / (rowHeight));
  if (row >= lines.size()){
    return m_textBuf.size();
  }

  // Find caret position of the first character in the clicked row
  size_t charNum = 0;
  for (size_t i = 0; i!= row; i++){
    charNum += lines[i].text.size();
  }

  // Find the clicked character and set the caret to the left or right
  // of it.
  charNum += caret_from_extents(info.CumulativeTextWidth(lines[row].text),
    pos, m_tri.P0().x);

  return std::min(charNum, m_textBuf.size());
}

Object* ObjText::Clone() const{
  return new ObjText(*this);
}

void ObjText::Draw(FaintDC& dc, ExpressionContext& ctx){
  TextInfoDC textInfo(m_settings);
  auto info = render_text(dc,
    m_textBuf,
    m_expression,
    m_tri,
    m_beingEdited,
    textInfo,
    ctx,
    m_settings);

  m_caret = info.caret;
  m_rowHeight = info.rowHeight;
}

void ObjText::DrawMask(FaintDC& dc){
  TextInfoDC textInfo(m_settings);
  render_text_mask(dc, m_textBuf, m_tri, textInfo, m_settings);
}

Rect ObjText::GetAutoSizedRect() const {
  TextInfoDC info(m_settings);
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
  for (const auto item : zip(tris, lines)){
    auto subPath(textInfo.GetTextPath(item.first, item.second.text));
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

text_lines_t split_evaluated(ExpressionContext& ctx,
  const ObjText* text)
{
  return split_string(TextInfoDC(text->GetSettings()),
    text->GetEvaluatedString(ctx),
    text->GetTri().Width());
}

Command* crop_text_region_command(ObjText* text){
  const Tri oldTri = text->GetTri();
  Tri tri(rotated(tri_from_rect(text->GetAutoSizedRect()),
      oldTri.GetAngle(), oldTri.P0()));
  return new TriCommand(text, New(tri), Old(oldTri), "Auto-Size");
}

bool is_text(const Object* obj){
  return dynamic_cast<const ObjText*>(obj) != nullptr;
}

} // namespace
