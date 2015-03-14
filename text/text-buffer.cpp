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
#include "text/char-constants.hh"
#include "text/text-buffer.hh"
#include "text/utf8-string.hh"

namespace faint{

CaretRange::CaretRange(Caret from, Caret to)
  : from(from),
    to(to)
{}

CaretRange CaretRange::Both(Caret v){
  return CaretRange(v,v);
}

bool CaretRange::Empty() const{
  return from == to;
}

bool CaretRange::operator==(const CaretRange& other) const{
  return from == other.from && to == other.to;
}

bool CaretRange::operator!=(const CaretRange& other) const{
  return !operator==(other);
}

TextBuffer::TextBuffer()
  : m_caret(0)
{
  m_sel.active = false;
  m_sel.origin = 0;
  m_sel.end = 0;
}

TextBuffer::TextBuffer(const utf8_string& text)
  : m_data(text),
    m_caret(0)
{
  m_sel.active = false;
  m_sel.origin = 0;
  m_sel.end = 0;
}

void TextBuffer::advance(bool select){
  if (!m_sel.active){
    m_sel.origin = m_caret;
  }
  m_sel.active = select;
  m_sel.end = std::min(m_caret + 1, m_data.size());
  m_caret = m_sel.end;
}

CaretRange TextBuffer::all() const{
  return CaretRange(0, m_data.size());
}

utf8_char TextBuffer::at(size_t pos) const{
  assert(pos < m_data.size());
  return m_data[pos];
}

Caret TextBuffer::caret() const{
  return m_caret;
}

void TextBuffer::caret(Caret caret, bool select){
  if (!m_sel.active){
    m_sel.origin = m_caret;
  }
  m_sel.active = select;
  m_sel.end = std::min(m_data.size(), caret);
  m_caret = m_sel.end;
}

void TextBuffer::clear(){
  m_data.clear();
  m_caret = 0;
  m_sel.active = false;
}

void TextBuffer::del(){
  if (m_sel.active){
    del_selection();
  }
  else {
    if (m_data.size() > m_caret){
      m_data.erase(m_caret,1);
    }
  }
}

void TextBuffer::del_back(){
  if (m_sel.active){
    del_selection();
  }
  else if (m_caret == 0) {
    return;
  }
  else {
    m_caret--;
    del();
  }
}

void TextBuffer::del_selection(){
  if (! m_sel.active){
    return;
  }
  m_data.erase(m_sel.min(), m_sel.num());
  m_caret = m_sel.min();
  m_sel.active = false;
  return;
}

void TextBuffer::devance(bool select){
  if (!m_sel.active){
    m_sel.origin = m_caret;
  }
  m_sel.active = select;

  if (m_caret == 0){
    return;
  }
  m_sel.end = m_caret - 1;
  m_caret = m_sel.end;
}

bool TextBuffer::empty() const{
  return m_data.empty();
}

const utf8_string& TextBuffer::get() const{
  return m_data;
}

CaretRange TextBuffer::get_sel_range() const{
  return m_sel.active ?
    CaretRange(m_sel.min(), m_sel.max()) :
    CaretRange(0,0);
}

utf8_string TextBuffer::get_selection() const{
  return m_sel.active ?
    m_data.substr(m_sel.min(), m_sel.num()) :
    utf8_string("");
}

void TextBuffer::insert(const utf8_char& c){
  del_selection();
  m_data.insert(m_caret, 1, c);
  m_caret+=1;
}

void TextBuffer::insert(const utf8_string& str){
  del_selection();
  m_data.insert(m_caret, str);
  m_caret += str.size();
}

void TextBuffer::move_down(bool select){
  size_t currLineStart = prev(eol);
  size_t x = m_caret - currLineStart;
  size_t pos = next(eol);
  if (pos == m_data.size()){
    caret(pos, select);
  }
  else{
    caret(std::min(pos + x, next(eol, pos + 1)),
      select);
  }
}

void TextBuffer::move_up(bool select){
  size_t currLineStart = prev(eol);
  if (currLineStart == 0){
    // Already on the first line, move to start of text
    caret(0, select);
    return;
  }
  size_t x = m_caret - currLineStart;
  size_t prevLineStart = prev(eol, currLineStart);
  caret(std::min(prevLineStart + x, currLineStart), select);
}

void TextBuffer::select_none(){
  m_sel.active = false;
}

void TextBuffer::select(const CaretRange& range){
  assert(range.from < m_data.size() + 1);
  assert(range.to < m_data.size() + 1);
  m_sel.origin = range.from;
  m_sel.end = range.to;
  m_caret = range.to;
  m_sel.active = true;
}


void TextBuffer::set(const utf8_string& s){
  m_data = s;
  select_none();
  m_caret = std::min(m_caret, m_data.size());

}

size_t TextBuffer::size() const{
  return m_data.size();
}

size_t TextBuffer::next(const utf8_char& c) const{
  return next(c, m_caret);
}

size_t TextBuffer::next(const utf8_char& c, size_t pos) const{
  size_t found = m_data.find(c, pos);
  if (found == utf8_string::npos){
    return m_data.size();
  }
  return found;
}

size_t TextBuffer::prev(const utf8_char& c) const{
  return prev(c, m_caret);
}

size_t TextBuffer::prev(const utf8_char& c, size_t pos) const{
  size_t found = m_data.rfind(c, pos - 1);
  if (found == utf8_string::npos){
    return 0;
  }
  return found;
}

bool word_delimiter(const utf8_char& ch){
  return is_punctuation(ch) ||
    ch == eol ||
    ch == left_parenthesis ||
    ch == right_parenthesis ||
    ch == space;
}

CaretRange word_boundaries(size_t pos, const TextBuffer& buffer){
  if (pos >= buffer.size()){
    return CaretRange::Both(buffer.size());
  }

  size_t left = 0;
  size_t right = buffer.size();
  if (word_delimiter(buffer.at(pos))){
    // Use the contiguous range of the delimiter at the given pos as
    // the boundaries.
    utf8_char ch = buffer.at(pos);
    for (size_t i = pos; i != 0; i--){
      if (buffer.at(i) != ch){
        left = i + 1;
        break;
      }
    }
    for (size_t i = pos; i != buffer.size(); i++){
      if (buffer.at(i) != ch){
        right = i;
        break;
      }
    }
  }
  else {
    // Find the delimiters and return the range of characters between
    // them.
    for (size_t i = pos; i != 0; i--){
      if (word_delimiter(buffer.at(i))){
        left = i + 1;
        break;
      }
    }
    for (size_t i = pos; i != buffer.size(); i++){
      if (word_delimiter(buffer.at(i))){
        right = i;
        break;
      }
    }
  }
  return CaretRange(left, right);
}

void transpose_chars(TextBuffer& buffer){
  const Caret caret = buffer.caret();
  const size_t numChars = buffer.size();
  if (caret == 0 || numChars == 1){
    return;
  }

  // Swap the characters separated by the caret, or the two characters
  // preceeding the caret if at the end
  const size_t offset = (caret == buffer.size() ? 1 : 0);
  Caret car1 = caret - offset;
  Caret car2 = caret - offset - 1;
  assert(car2 < car1);
  utf8_char ch1 = buffer.at(car1);
  utf8_char ch2 = buffer.at(car2);
  buffer.caret(car2);
  buffer.del();
  buffer.del();
  buffer.insert(ch1);
  buffer.insert(ch2);

  // Move past the swapped characters
  buffer.caret(caret + 1 - offset);
}

} // namespace
