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

#ifndef FAINT_AUTO_COMPLETE_HH
#define FAINT_AUTO_COMPLETE_HH
#include <vector>
#include "text/utf8-string.hh"

namespace faint {

class ACNode;
class Words;

class AutoComplete{
  // Dictionary for auto-completion of words.
public:
  AutoComplete();
  AutoComplete(const std::vector<utf8_string>&);
  ~AutoComplete();

  // Add a word to recognize in match()
  void add(const utf8_string& word);

  // Get all words matching the string
  Words match(const utf8_string&);

  AutoComplete(const AutoComplete&) = delete;
  AutoComplete& operator=(const AutoComplete&) = delete;
private:
  std::vector<ACNode*> m_nodes;
};

class WordsImpl;

class Words{
  // A list of words retrieved from an AutoComplete object by matching
  // a partial word.
public:
  Words();
  Words(const Words&);
  ~Words();
  Words& operator=(const Words&);
  void clear();
  bool empty() const;
  utf8_string get(size_t) const;
  size_t size() const;

private:
  Words(WordsImpl* m_impl);
  WordsImpl* m_impl;
  friend class AutoComplete;
};

class AutoCompleteState{
  // Stateful auto-completion, for cycling through alternative
  // matches.
  //
  // Complete(str) initializes with words matching str.
  //
  // Prev() and Next() successively returns the previous/next word,
  // with wrap-around. They must not be called if Empty().
  //
  // Completing with a new word or calling Forget() clears the current
  // alternatives.
public:
  explicit AutoCompleteState(AutoComplete&);

  utf8_string Complete(const utf8_string&);
  void Forget();
  utf8_string Get() const;
  bool Empty() const;
  bool Has() const;
  utf8_string Next();
  utf8_string Prev();

  AutoCompleteState& operator=(const AutoCompleteState&) = delete;
private:
  AutoComplete& m_ac;
  Words m_words;
  size_t m_index;
};

} // Namespace

#endif
