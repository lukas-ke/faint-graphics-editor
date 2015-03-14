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

#include <algorithm>
#include <cassert>
#include "text/auto-complete.hh"
#include "text/char-constants.hh"

namespace faint {

class ACNode;

struct ACNodeCmp{
  bool operator()(const ACNode*, const ACNode*) const;
};

class WordsImpl{
public:
  ACNode* m_node;
  utf8_string m_base;
};


class ACNode {
public:
  ACNode(const utf8_char& c)
    : m_char(c)
  {}

  ~ACNode(){
    for (auto node : m_children){
      delete node;
    }
  }

  void extend(const utf8_string& word){
    if (!word.empty()){
      ACNode* node = add(word[0]);
      node->extend(word.substr(1, word.size() - 1));
    }
    else {
      add(full_stop);
    }
  }

  ACNode* add(const utf8_char& c){
    for (size_t i = 0; i != m_children.size(); i++){
      if (m_children[i]->m_char == c){
        return m_children[i];
      }
    }
    ACNode* node = new ACNode(c);
    m_children.push_back(node);
    sort(begin(m_children), end(m_children), ACNodeCmp());
    return node;
  }

  ACNode* find(const utf8_string& str){
    if (str.size() == 1 && str[0] == m_char){
      return this;
    }
    if (str.size() > 1){
      if (m_char != str[0]){
        return nullptr;
      }
      for (size_t i = 0; i != m_children.size(); i++){
        ACNode* node = m_children[i]->find(str.substr(1, str.size() - 1));
        if (node != nullptr){
          return node;
        }
      }
    }
    return nullptr;
  }

  void all_words(std::vector<utf8_string>& collect, const utf8_string& prepend){
    if (m_children.empty()){
      collect.push_back(prepend);
      return;
    }
    else {
      for (size_t i = 0; i != m_children.size(); i++){
        std::vector<utf8_string> subwords;
        m_children[i]->all_words(subwords, prepend + m_char);
        for (const auto& sw : subwords){
          collect.push_back(sw);
        }
      }
    }
  }

  bool operator<(const ACNode& other) const {
    return other.m_char < m_char;
  }

  utf8_char m_char;
  std::vector<ACNode*> m_children;
};


AutoComplete::AutoComplete(){
}

AutoComplete::AutoComplete(const std::vector<utf8_string>& v){
  for (const auto& word : v){
    add(word);
  }
}

AutoComplete::~AutoComplete(){
  for (size_t i = 0; i != m_nodes.size(); i++){
    delete m_nodes[i];
  }
}

void AutoComplete::add(const utf8_string& word){
  for (size_t i = 0; i != m_nodes.size(); i++){
    if (m_nodes[i]->m_char == word[0]){
      m_nodes[i]->extend(word.substr(1, word.size() - 1));
      return;
    }
  }

  ACNode* node = new ACNode(word[0]);
  m_nodes.push_back(node);
  node->extend(word.substr(1, word.size() - 1));
}

Words AutoComplete::match(const utf8_string& str){
  WordsImpl* w = new WordsImpl();
  w->m_node = nullptr;
  for (size_t i = 0; i != m_nodes.size(); i++){
    ACNode* node = m_nodes[i];
    if (node->m_char == str[0]){
      ACNode* found = node->find(str);
      if (found != nullptr){
        w->m_node = found;
        w->m_base = str;
        break;
      }
    }
  }
  return Words(w);
}

bool ACNodeCmp::operator()(const ACNode* lhs, const ACNode* rhs) const {
  return !(*lhs < *rhs);
}

Words::Words(){
  m_impl = new WordsImpl();
  m_impl->m_node = nullptr;
}

Words::Words(WordsImpl* impl)
  : m_impl(impl)
{}

Words::~Words(){
  delete m_impl;
}

Words::Words(const Words& other){
  m_impl = new WordsImpl();
  m_impl->m_node = other.m_impl->m_node;
  m_impl->m_base = other.m_impl->m_base;
}

Words& Words::operator=(const Words& other){
  if (&other == this){
    return *this;
  }
  m_impl = new WordsImpl();
  m_impl->m_node = other.m_impl->m_node;
  m_impl->m_base = other.m_impl->m_base;
  return *this;
}

bool Words::empty() const{
  return size() == 0;
}

size_t Words::size() const{
  if (m_impl->m_node == nullptr){
    return 0;
  }

  std::vector<utf8_string> words;
  const utf8_string& base = m_impl->m_base;
  m_impl->m_node->all_words(words, base.substr(0, base.size() - 1));
  return words.size();
}

utf8_string Words::get(size_t i) const{
  assert(m_impl->m_node != nullptr);
  std::vector<utf8_string> words;
  const utf8_string& base = m_impl->m_base;
  m_impl->m_node->all_words(words, base.substr(0, base.size() - 1));
  return words[i];
}

void Words::clear(){
  m_impl->m_base = "";
  m_impl->m_node = nullptr;
}

AutoCompleteState::AutoCompleteState(AutoComplete& ac)
  : m_ac(ac),
    m_index(0)
{}

utf8_string AutoCompleteState::Complete(const utf8_string& part){
  m_index = 0;
  m_words = m_ac.match(part);
  return m_words.empty() ? part : m_words.get(0);
}

void AutoCompleteState::Forget(){
  m_words.clear();
}

utf8_string AutoCompleteState::Get() const{
  return m_words.empty() ? "" :
    m_words.get(m_index);
}

bool AutoCompleteState::Empty() const{
  return m_words.empty();
}

bool AutoCompleteState::Has() const{
  return !m_words.empty();
}

utf8_string AutoCompleteState::Next(){
  assert(!Empty());
  m_index = (m_index + 1) % m_words.size();
  return m_words.get(m_index);
}

utf8_string AutoCompleteState::Prev(){
  assert(!Empty());
  m_index = (m_index == 0) ?
    m_words.size() - 1 :
    m_index - 1;
  return m_words.get(m_index);
}

} // Namespace
