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
#include "text/slice.hh"
#include "text/split-string.hh"
#include "util/optional.hh"

namespace faint{

static utf8_string split_word(const TextInfo& info,
  const utf8_string& string,
  text_lines_t& result)
{
  // Just break the word in half.
  const utf8_string half(slice_up_to(string, string.size() / 2));
  const coord width = info.GetWidth(half);
  result.push_back(TextLine::SoftBreak(width, half));
  return slice_from(string, string.size() / 2);
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
    wordEnd = string.find(chars::space, wordStart);
    if (wordEnd == std::string::npos){
      wordEnd = string.size();
    }
    utf8_string word = slice(string, wordStart, wordEnd);
    const coord width = info.GetWidth(line + chars::space + word);
    if (!line.empty() && width > maxWidth){
      result.push_back(TextLine::SoftBreak(width, line + chars::space));
      line.clear();
    }

    if (info.GetWidth(word) > maxWidth) {
      word = split_word(info, word, result);
    }

    if (!line.empty()){
      line += chars::space;
    }

    line += word;
    wordStart = wordEnd + 1;
  } while (wordEnd != string.size());

  if (line.size() > 1){
    const utf8_string last(line + chars::space);
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
    lineEnd = string.find(chars::eol, lineStart);
    bool softBreak = lineEnd == std::string::npos;
    if (softBreak){
      lineEnd = string.size();
    }

    const coord width = info.GetWidth(slice(string, lineStart, lineEnd));
    if (maxWidth.NotSet() || width < maxWidth.Get()){
      if (softBreak){
        result.push_back(TextLine::SoftBreak(width,
          slice(string, lineStart, lineEnd) + chars::space));
        lineStart = lineEnd + 1;
      }
      else {
        result.push_back(TextLine::HardBreak(width,
          slice(string, lineStart, lineEnd) + chars::space));
        lineStart = lineEnd + 1;
      }
    }
    else {
      split_line(info, slice(string, lineStart, lineEnd),
        maxWidth.Get(), result);
      lineStart = lineEnd;
    }
  } while (lineEnd != string.size());

  if (!result.empty()){
    // Remove trailing space from last line
    auto& last = result.back().text;
    last = slice_up_to(last, -1);
  }
  return result;
}

} // namespace
