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

#ifndef FAINT_TEXT_BUFFER_HH
#define FAINT_TEXT_BUFFER_HH
#include <algorithm>
#include "text/utf8-string.hh"

namespace faint{

using Caret = size_t;

// Selection range
class CaretRange{
public:
  CaretRange(Caret from, Caret to);
  static CaretRange Both(Caret);
  bool Empty() const;
  bool operator==(const CaretRange&) const;
  bool operator!=(const CaretRange&) const;
  Caret from;
  Caret to;
};

class TextBuffer{
  // A buffer for text entry with caret and selection
  // handling.
  //
  // The caret position is zero based:
  //   "|Hello world" = 0
  //   "H|ello world" = 1
  //   "Hello world|" = size() = 11
  //
  // The selection is similar to having two carets:
  // "H|ell]o world" = 1->4
public:
  TextBuffer();
  explicit TextBuffer(const utf8_string&);
  void advance(bool select=false);
  CaretRange all() const;
  utf8_char at(size_t) const;
  bool empty() const;
  Caret caret() const;
  void caret(Caret, bool select=false);
  void clear();
  void del();
  void del_back();
  void devance(bool select=false);
  const utf8_string& get() const;
  CaretRange get_sel_range() const;
  utf8_string get_selection() const;
  void insert(const utf8_char&);
  void insert(const utf8_string&);
  void move_down(bool select=false);
  void move_up(bool select=false);
  size_t next(const utf8_char&) const;
  size_t next(const utf8_char&, size_t pos) const;
  size_t prev(const utf8_char&) const;
  size_t prev(const utf8_char&, size_t pos) const;
  void select_none();
  void select(const CaretRange&);
  void set(const utf8_string&);
  size_t size() const;
private:
  void del_selection();
  struct{
    bool active;
    Caret origin;
    Caret end;

    inline Caret min() const{
      return std::min(origin, end);
    }
    inline Caret max() const{
      return std::max(origin, end);
    }
    inline Caret num() const{
      return max() - min();
    }
  } m_sel;

  utf8_string m_data;
  Caret m_caret;
};

// Finds the boundaries of the word encompassing the position
CaretRange word_boundaries(size_t, const TextBuffer&);
void transpose_chars(TextBuffer&);

} // namespace

#endif
