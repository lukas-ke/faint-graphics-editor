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

#include "rendering/faint-dc.hh"
#include "text/text-line.hh" // TextInfo
#include "text/text-buffer.hh"
#include "text/text-expression-context.hh"
#include "text/text-expression.hh"
#include "util/settings.hh"

namespace faint{

static utf8_string get_expression_string(const parse_result_t& result,
  const ExpressionContext& context);

struct TextRenderInfo{
  int rowHeight;
  LineSegment caret;
};

TextRenderInfo render_text(FaintDC&,
  const TextBuffer&,
  const Optional<parse_result_t>&,
  const Tri&,
  bool currentlyEdited,
  TextInfo&,
  ExpressionContext&,
  const Settings&);

void render_text_mask(
  FaintDC&,
  const TextBuffer&,
  const Tri&,
  TextInfo&,
  const Settings&);

} // namespace
