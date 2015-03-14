// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_DISTINCT_HH
#define FAINT_DISTINCT_HH
#include <utility> // for std::move

namespace faint{

template<typename T, typename CATEGORY, int ID>
class Distinct{
  // Promoter of parameter incompatibility by deriving distinct types.
  //
  // T is the wrapped type, CATEGORY provides further separation
  // than the ID.
  //
  // Example,
  // Avoid confusing the message and the title in message boxes
  //   class category_message;
  //   using Title = Distinct<std::string, category_message, 0>;
  //   using Message = Distinct<std::string, category_message, 1>;
  //   void Message(const Title&, const Message&);
public:
  explicit Distinct(const T& obj) :
    m_obj(obj)
  {}

  Distinct(T&& obj) :
    m_obj(std::move(obj))
  {}

  T& Get(){
    return m_obj;
  }

  const T& Get() const{
    return m_obj;
  }

  bool operator==(const Distinct<T, CATEGORY, ID>& other) const{
    return m_obj == other.m_obj;
  }

  using value_type = T;
private:
  T m_obj;
};

// Lesser-promoter of parameter incompatibility.  T is the wrapped
// type, the ID provides distinction (without the categorizing of the
// Distinct-template).
class category_common;
template<typename T, int ID>
using LessDistinct = Distinct<T, category_common, ID>;

template <typename T>
struct Order{
  // Types for parameter ordering, when two same-type parameters
  // represent something new and something old, to avoid mixing them
  // up.
  //
  // For instantiating, use the New and Old function templates outside
  // this struct
  using New = Distinct<T, Order, 0>;
  using Old = Distinct<T, Order, 1>;
};

template<class T, class CATEGORY, int ID>
class Subtype : public T {
  // Same idea as the Distinct template, but retaining the base
  // types interface.
  //
  // Like Distinct, this prevents passing a T when a Subtype<T> is
  // required, courtesy of the explicit constructors.
  //
  // On the other hand, a SubType<T> can be passed where a T is
  // required.
public:
  explicit Subtype(const T& val) :
    T(val)
  {}

  template<class ...Args>
  explicit Subtype(const Args&... args)
    : T(args...)
  {}

  using base_type = T;
};

template<class T, class C1, class C2, int ID1, int ID2>
void assert_same(const Subtype<T, C1, ID1>&,
  const Subtype<T, C2, ID2>&)
{
  static_assert(std::is_same<C1, C2>::value && ID1 == ID2,
    "Comparing apples to oranges.");
}

template<class T, class C, int ID>
const T& raw(const Subtype<T, C, ID>& v){
  return static_cast<const T&>(v);
}

class category_default_to;
template<typename T>
using Default = Distinct<T, category_default_to, 0>;

template<typename T>
Default<T> default_to(const T& value){
  return Default<T>(value);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator==(const Subtype<T, C1, ID1>& v1,
  const Subtype<T, C2, ID2>& v2)
{
  assert_same(v1, v2);
  return raw(v1) == raw(v2);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator!=(const Subtype<T, C1, ID1>& v1,
  const Subtype<T, C2, ID2>& v2)
{
  assert_same(v1, v2);
  return raw(v1) != raw(v2);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator<(const Subtype<T, C1, ID1>& v1, const Subtype<T, C2, ID2>& v2){
  static_assert(std::is_same<C1, C2>::value && ID1 == ID2,
    "Comparing apples to oranges.");
  return raw(v1) < raw(v2);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator<=(const Subtype<T, C1, ID1>& v1, const Subtype<T, C2, ID2>& v2){
  static_assert(std::is_same<C1, C2>::value && ID1 == ID2,
    "Comparing apples to oranges.");
  return static_cast<const T&>(v1) <= static_cast<const T&>(v2);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator>(const Subtype<T, C1, ID1>& v1,
  const Subtype<T, C2, ID2>& v2)
{
  assert_same(v1, v2);
  return raw(v1) > raw(v2);
}

template<class T, class C1, class C2, int ID1, int ID2>
bool operator>=(const Subtype<T, C1, ID1>& v1,
  const Subtype<T, C2, ID2>& v2)
{
  assert_same(v1, v2);
  return raw(v1) >= raw(v2);
}

template<typename T>
typename Order<T>::Old Old(const T& t){
  return typename Order<T>::Old(t);
}

template<typename T>
typename Order<T>::New New(const T& t){
  return typename Order<T>::New(t);
}

class category_alternative;
template<typename T>
using Alternative = Distinct<T, category_alternative, 0>;

template<typename T>
Alternative<T> alternate(const T& t){
  return Alternative<T>(t);
}

template <typename T>
struct Boundary{
  using Min = Distinct<T, Boundary, 0>;
  using Max = Distinct<T, Boundary, 1>;
};

template<typename T>
typename Boundary<T>::Min Min(const T& t){
  return typename Boundary<T>::Min(t);
}

template<typename T>
typename Boundary<T>::Max Max(const T& t){
  return typename Boundary<T>::Max(t);
}

template<typename T>
class from_t{
  // Class for wrapping a container function parameter with a
  // from-preposition. Used with the from function template.
  //
  // Example:
  //   remove(someThings, from(myContainer);
public:
  explicit from_t(T& container) :
    m_container(container)
  {}

  T& Get() const{
    return m_container;
  }

  from_t<T>& operator=(const from_t<T>&) = delete;

private:
  T& m_container;
};

template<typename T>
from_t<T> from(T& container){
  return from_t<T>(container);
}

template<typename T>
class these_t{
public:
  explicit these_t(T& container) :
    m_container(container)
  {}

  T& Get() const{
    return m_container;
  }

  these_t<T>& operator=(const these_t<T>&) = delete;
private:
  T& m_container;
};

template<typename T>
these_t<T> these(T& container){
  return these_t<T>(container);
}
template<typename T>
class its_yours_t{
public:
  explicit its_yours_t(T* obj) : m_obj(obj)
  {}
  T* Get() const{
    return m_obj;
  }
private:
  T* m_obj;
};

template<typename T>
class just_a_loan_t{
public:
  explicit just_a_loan_t(T* obj) : m_obj(obj)
  {}
  T* Get() const{
    return m_obj;
  }
private:
  T* m_obj;
};

template<typename T>
its_yours_t<T> its_yours(T* ptr){
  return its_yours_t<T>(ptr);
}

template<typename T>
just_a_loan_t<T> just_a_loan(T* ptr){
  return just_a_loan_t<T>(ptr);
}

} // namespace

#endif
