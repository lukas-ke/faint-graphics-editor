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

#include "text/utf8-string.hh"
#include "text/utf8.hh"

namespace faint{

utf8_string::utf8_string(size_t n, const utf8_char& ch){
  for (size_t i = 0; i != n; i++){
    m_data += ch.str();
  }
}

utf8_string::utf8_string(const utf8_char& ch)
  : utf8_string(1, ch)
{}

utf8_string::utf8_string(const char* str)
  : m_data(str)
{}

utf8_string::utf8_string(const std::string& str)
  : m_data(str)
{}

size_t utf8_string::bytes() const{
  return m_data.size();
}

void utf8_string::clear() {
  m_data.clear();
}

utf8_string utf8_string::substr(size_t pos, size_t n) const{
  size_t startByte = utf8::char_num_to_byte_num(pos, m_data);
  size_t numBytes = (n == utf8_string::npos) ?
    std::string::npos : utf8::char_num_to_byte_num(pos + n, m_data) - startByte;

  return utf8_string(m_data.substr(startByte, numBytes));
}

const char* utf8_string::c_str() const{
  return m_data.c_str();
}

const std::string& utf8_string::str() const{
  return m_data;
}

size_t utf8_string::size() const{
  return utf8::num_characters(m_data);
}

bool utf8_string::empty() const{
  return m_data.empty();
}

utf8_string& utf8_string::erase(size_t pos, size_t n){
  size_t startByte = utf8::char_num_to_byte_num(pos, m_data);

  size_t numBytes = (n == npos ? npos :
    utf8::char_num_to_byte_num(pos + n, m_data) - startByte);
  m_data.erase(startByte, numBytes);
  return *this;
}

utf8_string& utf8_string::insert(size_t pos, const utf8_string& inserted){
  m_data.insert(utf8::char_num_to_byte_num(pos, m_data), inserted.str());
  return *this;
}

utf8_string& utf8_string::insert(size_t pos, size_t num, const utf8_char& c){
  insert(pos, utf8_string(num, c));
  return *this;
}

utf8_char utf8_string::operator[](size_t i) const{
  size_t pos = utf8::char_num_to_byte_num(i, m_data);
  size_t numBytes = faint::utf8::prefix_num_bytes(m_data[pos]);
  return utf8_char(m_data.substr(pos, numBytes));
}

size_t utf8_string::find(const utf8_char& ch, size_t start) const{
  // Since the leading byte has a unique pattern, using regular
  // std::string find should be OK, I think.
  size_t pos = m_data.find(ch.str(), utf8::char_num_to_byte_num(start, m_data));
  if (pos == npos){
    return pos;
  }
  return utf8::byte_num_to_char_num(pos, m_data);
}

size_t utf8_string::rfind(const utf8_char& ch, size_t start) const{
  // Since the leading byte has a unique pattern, using regular
  // std::string rfind should be OK, I think.
  if (m_data.empty()){
    return npos;
  }

  size_t startByte = (start == npos) ? m_data.size() - 1 :
    utf8::char_num_to_byte_num(start, m_data);
  size_t pos = m_data.rfind(ch.str(), startByte);
  if (pos == npos){
    return pos;
  }
  return pos == npos ? npos :
    utf8::byte_num_to_char_num(pos, m_data);
}

utf8_string& utf8_string::operator=(const utf8_string& other){
  if (&other == this){
    return *this;
  }
  m_data = other.m_data;
  return *this;

}
utf8_string& utf8_string::operator+=(const utf8_char& ch){
  m_data += ch.str();
  return *this;
}

utf8_string& utf8_string::operator+=(const utf8_string& str){
  m_data += str.str();
  return *this;
}

utf8_string operator+(const utf8_string& lhs, const utf8_char& rhs){
  return utf8_string(lhs.str() + rhs.str());
}

utf8_string operator+(const utf8_string& lhs, const utf8_string& rhs){
  return utf8_string(lhs.str() + rhs.str());
}

utf8_string operator+(const utf8_char& lhs, const utf8_string& rhs){
  return utf8_string(lhs.str() + rhs.str());
}

const size_t utf8_string::npos(std::string::npos);

bool utf8_string::operator<(const utf8_string& s) const{
  return m_data < s.m_data;
}

bool is_ascii(const utf8_string& s){
  const std::string& bytes = s.str();
  for (char ch : bytes){
    if (utf8::prefix_num_bytes(ch) != 1){
      return false;
    }
  }
  return true;
}

std::ostream& operator<<(std::ostream& o, const utf8_string& s){
  o << s.str();
  return o;
}

bool operator==(const utf8_string& lhs, const utf8_string& rhs){
  return lhs.str() == rhs.str();
}

bool operator!=(const utf8_string& lhs, const utf8_string& rhs){
  return !(lhs == rhs);
}

utf8_string_const_iterator begin(const utf8_string& s){
  return utf8_string_const_iterator(s, 0);
}

utf8_string_const_iterator end(const utf8_string& s){
  return utf8_string_const_iterator(s, s.size());
}

} // namespace
