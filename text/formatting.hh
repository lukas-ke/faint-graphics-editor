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

#ifndef FAINT_FORMATTING_HH
#define FAINT_FORMATTING_HH
#include <functional>
#include <vector>
#include "text/utf8-string.hh"
#include "util/common-fwd.hh"
#include "util/template-fwd.hh"

namespace faint{

class Precision{
public:
  explicit constexpr Precision(int decimals) : m_decimals(decimals)
  {}

  Precision(float) = delete;

  operator int() const{
    return m_decimals;
  }

private:
  int m_decimals;
};

inline Precision operator "" _dec(unsigned long long decimals){
  return Precision(static_cast<int>(decimals));
}

utf8_string bracketed(const utf8_string&);
utf8_string capitalized(const utf8_string&);
utf8_string decapitalized(const utf8_string&);
utf8_string lbl(const utf8_string& label, const utf8_string& value);
utf8_string lbl(const utf8_string& label, int value);
utf8_string lbl_u(const utf8_string& label, size_t value);

// Returns "<ModifierKey>=action"
utf8_string primary_modifier(const utf8_string& action);
utf8_string secondary_modifier(const utf8_string& action);
utf8_string both_modifiers(const utf8_string& action);
utf8_string both_modifiers();

utf8_string quoted(const utf8_string&);
utf8_string str(const IntPoint&);
utf8_string str(const IntSize&);
utf8_string str(const Size&, const Precision&);
utf8_string str(const Point&);

// Returns the index one-based as a string (i.e. offset +1).
utf8_string str_user(const Index&);

// Returns the index zero-based as a string.
utf8_string str_coder(const Index&);

utf8_string str(const Paint&);
utf8_string str(const Scale&);
utf8_string str_axis_adverb(Axis);
utf8_string str_degrees(const Angle&);
utf8_string str_degrees_symbol(const Angle&);
utf8_string str_degrees_int_symbol(int);
utf8_string str_center_radius(const Point&, coord r);
utf8_string str_center_radius(const Point&, const Radii&);
utf8_string str(coord value, const Precision&);
utf8_string str_from_to(const IntPoint&, const IntPoint&);
utf8_string str_from_to(const Point&, const Point&);
utf8_string str_hex(const Color&);
utf8_string str_hex(const int);
utf8_string str_interval(const Interval&);
utf8_string str_int_length(int);
utf8_string str_length(coord);
utf8_string str_line_status(const IntLineSegment&);
utf8_string str_line_status_subpixel(const LineSegment&);
utf8_string str_percentage(int numerator, int denominator);
utf8_string str_range(const ClosedIntRange&);


utf8_string str_rgb(const Color&);
utf8_string str_rgba(const Color&);

// Excludes the alpha-component if fully opaque
utf8_string str_smart_rgba(const Color&);

class category_formatting;
using rgb_prefix = Distinct<bool, category_formatting, 0>;
utf8_string str_smart_rgba(const Color&, const rgb_prefix&);

utf8_string str_int(int);
utf8_string str_int_lpad(int, int w); // Fixme: Use a type for the padding, with _pad or smth

utf8_string str_uint(size_t);
utf8_string str_uint(int) = delete;

class StrBtn{
// Expresses a mouse button and its opposite as strings.
public:
  StrBtn(MouseButton);
  const utf8_string This(bool capital) const;
  const utf8_string Other(bool capital) const;
private:
  utf8_string m_btnThis;
  utf8_string m_btnThat;
};

inline utf8_string join(const utf8_string& /*sep*/){
  return utf8_string("");
}

inline utf8_string concat(const utf8_string& lhs, const utf8_string& rhs){
  return lhs + rhs;
}

inline utf8_string concat(const utf8_string& lhs, const utf8_char& rhs){
  return lhs + rhs;
}

inline utf8_string concat(const utf8_string& lhs, const std::string& rhs){
  return lhs + utf8_string(rhs);
}

inline utf8_string concat(const utf8_string& lhs, const char* rhs){
  return lhs + utf8_string(rhs);
}

inline utf8_string concat(const char* lhs, const utf8_string& rhs){
  return utf8_string(lhs) + rhs;
}

inline utf8_string concat(const char* lhs, const char* rhs){
  return utf8_string(lhs) + utf8_string(rhs);
}

template<typename T>
T join(const utf8_string& /*sep*/, const T terminal){
  return terminal;
}

inline utf8_string join(const utf8_string& /*sep*/, const char* terminal){
  return terminal;
}

// Join the strings, separated by the first argument.
template<class A, class ...B>
utf8_string join(const utf8_string& sep, const A& head, const B&... tail){
  return concat(concat(head, sep), join(sep, tail...));
}

// Join all strings, separated by ", "
template<class A, class ...B>
utf8_string comma_sep(const A& head, const B&... tail){
  return join(utf8_string(", "), head, tail...);
}

// Join all strings, separated by endline (\n)
template<class A, class ...B>
utf8_string endline_sep(const A& head, const B&... tail){
  return join(utf8_string("\n"), head, tail...);
}

// Join all strings, without separator.
template<class A, class ...B>
utf8_string no_sep(const A& head, const B&... tail){
  return join(utf8_string(""), head, tail...);
}

// Join all strings, separated by space.
template<class A, class ...B>
utf8_string space_sep(const A& head, const B&... tail){
  return join(utf8_string(" "), head, tail...);
}

// join-variants taking a vector.
utf8_string join(const utf8_string& sep, const std::vector<utf8_string>&);
utf8_string comma_sep(const std::vector<utf8_string>&);
utf8_string endline_sep(const std::vector<utf8_string>&);
utf8_string no_sep(const std::vector<utf8_string>&);

// Converts all objects in the container to strings using func, and
// concatenates them separated by the specified separator
template<typename T>
utf8_string join_fn(const utf8_string& separator, T container, const std::function<utf8_string(const typename T::value_type&)>& func){
  if (container.empty()){
    return utf8_string("");
  }

  typename T::const_iterator it = begin(container);
  utf8_string result = func(*it);
  ++it;
  while (it != end(container)){
    result += separator;
    result += func(*it);
    ++it;
  }
  return result;
}

class Sentence{
public:
  explicit Sentence(const utf8_string& text)
    : m_text(text + ".")
  {}

  template<class A, class ...B>
  explicit Sentence(const A& head, const B&... tail)
    : m_text(join(utf8_string(" "), head, tail...) + ".")
  {}

  Sentence operator+(const utf8_string& text) const{
    return Sentence(m_text + " " + text);
  }

  operator utf8_string() const{
    return m_text;
  }

private:
  utf8_string m_text;
};

} // namespace

#endif
