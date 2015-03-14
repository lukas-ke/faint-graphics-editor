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

#include "text/char-constants.hh"
#include "text/split-string.hh"
#include "util/optional.hh"

namespace faint{

#ifdef DEBUG_SPLIT_STRING
const utf8_char wordBrk('>');
const utf8_char& hardBrk = pilcrow_sign;
const utf8_char& softBrk = downwards_arrow_with_tip_leftwards;
#else
const utf8_char& wordBrk = space;
const utf8_char& hardBrk = space;
const utf8_char& softBrk = space;
#endif

static utf8_string split_word(const TextInfo& info,
  const utf8_string& string,
  text_lines_t& result)
{
  // Just break the word in half.
  const utf8_string half(string.substr(0, string.size() / 2) + wordBrk);
  const coord width = info.GetWidth(half);
  result.push_back(TextLine::SoftBreak(width, half));
  return string.substr(string.size() / 2, string.size() - string.size() / 2);
}

// Split a line at suitable positions to make it shorter than
// maxWidth. The line should not contain embedded line breaks.
static void split_line(const TextInfo& info,
  const utf8_string& string,
  coord maxWidth, text_lines_t& result)
{
  size_t wordStart = 0;
  size_t wordEnd = 0;

  utf8_string line;
  do {
    wordEnd = string.find(utf8_char(" "), wordStart);
    if (wordEnd == std::string::npos){
      wordEnd = string.size();
    }
    utf8_string word = string.substr(wordStart, wordEnd - wordStart);
    const coord width = info.GetWidth(line + space + word);
    if (!line.empty() && width > maxWidth){
      result.push_back(TextLine::SoftBreak(width, line + softBrk));
      line.clear();
    }

    if (info.GetWidth(word) > maxWidth) {
      word = split_word(info, word, result);
    }

    if (!line.empty()){
      line += space;
    }

    line += word;
    wordStart = wordEnd + 1;
  } while (wordEnd != string.size());

  if (line.size() > 1){
    const utf8_string last(line + softBrk);
    const coord width = info.GetWidth(last);
    result.push_back(TextLine::SoftBreak(width, last));
  }
}

text_lines_t split_string(const TextInfo& info,
  const utf8_string& string,
  const max_width_t& maxWidth)
{
  size_t lineEnd = 0;
  size_t lineStart = 0;

  text_lines_t result;
  do {
    lineEnd = string.find(eol, lineStart);
    bool softBreak = lineEnd == std::string::npos;
    if (softBreak){
      lineEnd = string.size();
    }

    const coord width = info.GetWidth(string.substr(lineStart,
      lineEnd - lineStart));
    if (maxWidth.NotSet() || width < maxWidth.Get()){
      if (softBreak){
        result.push_back(TextLine::SoftBreak(width,
          string.substr(lineStart, lineEnd - lineStart) + hardBrk));
      }
      else {
        result.push_back(TextLine::HardBreak(width,
          string.substr(lineStart, lineEnd - lineStart) + hardBrk));
      }
    }
    else {
      split_line(info, string.substr(lineStart, lineEnd - lineStart),
        maxWidth.Get(), result);
    }

    lineStart = lineEnd + 1;

  } while (lineEnd != string.size());
  return result;
}

} // namespace
