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

#include "text/text-line.hh"

namespace faint{

TextLine TextLine::SoftBreak(coord w, const utf8_string& str){
  return TextLine(false, w, str);
}

TextLine TextLine::HardBreak(coord w, const utf8_string& str){
  return TextLine(true, w, str);
}

TextLine::TextLine(bool hardBreak, coord width, const utf8_string& text)
  : hardBreak(hardBreak),
    text(text),
    width(width)
{}

} // namespace
