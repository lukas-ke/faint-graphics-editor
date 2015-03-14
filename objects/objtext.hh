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

#ifndef FAINT_OBJTEXT_HH
#define FAINT_OBJTEXT_HH
#include "geo/line.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "text/text-buffer.hh"
#include "text/text-expression.hh"
#include "text/text-line.hh"

namespace faint{

class Command;

class ObjText : public Object{
public:
  ObjText(const Tri&, const utf8_string&, const Settings&);
  Object* Clone() const override;
  void Draw(FaintDC&, ExpressionContext&) override;
  void DrawMask(FaintDC&) override;

  std::vector<PathPt> GetPath(const ExpressionContext&) const override;
  IntRect GetRefreshRect() const override;
  Tri GetTri() const override;
  utf8_string GetType() const override;
  bool ShowSizeBox() const override;

  // Non virtual
  // The distance from the top-left (quoi?) to the base-line
  coord BaselineOffset() const;
  size_t CaretPos(const Point&) const;
  LineSegment GetCaret() const;
  Rect GetAutoSizedRect() const;
  utf8_string GetRawString() const;
  utf8_string GetEvaluatedString(const ExpressionContext&) const;
  TextBuffer& GetTextBuffer();

  bool HasSelectedRange() const;
  coord RowHeight() const;
  void SetCaretPos(size_t, bool select);
  void SetEdited(bool);
  void SetTri(const Tri&) override;
private:
  ObjText(const ObjText&); // For Clone
  void Init();
  LineSegment ComputeCaret(const TextInfo&, const Tri&, const text_lines_t&);
  TextBuffer m_textBuf;
  bool m_beingEdited;
  LineSegment m_caret;
  mutable coord m_rowHeight;
  mutable int m_lastFontSize;
  mutable utf8_string m_lastFontFace;
  Settings m_highlightSettings;
  Tri m_tri;
  Optional<parse_result_t> m_expression;
};

text_lines_t split_evaluated(ExpressionContext&,
  const ObjText*);

Command* crop_text_region_command(ObjText* object);

bool is_text(const Object*);

} // namespace

#endif
