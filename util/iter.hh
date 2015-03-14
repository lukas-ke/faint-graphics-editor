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

#ifndef FAINT_ITER_HH
#define FAINT_ITER_HH
#include <cassert>
#include "util/common-fwd.hh"

namespace faint{

template<typename T>
class ContainerType{
  // The non-specialized ContainerType stores a container by
  // reference, as the container is assumed to live past the iteration
  // scope.
  // The specializations for wrappers (e.g. but_first_t) instead store
  // wrappers by value, as they would expire immediately otherwise.
public:
  ContainerType(const T& container)
    : c(container)
  {}
  const T& c;
  ContainerType& operator=(const ContainerType&) = delete;
};

template<typename Container>
class reversed_t {
private:
  ContainerType<Container> m_container;
public:
  reversed_t(const Container& c)
    : m_container(c)
  {}

  auto begin() const -> decltype(m_container.c.rbegin()) {
    return m_container.c.rbegin();
  }

  auto end() const -> decltype(m_container.c.rend()) {
    return m_container.c.rend();
  }
  using value_type = typename Container::value_type;
  reversed_t& operator=(const reversed_t&) = delete;
};


template<typename T>
class ContainerType<reversed_t<T> >{
public:
  ContainerType(const reversed_t<T>& container)
    : c(container)
  {}
  const reversed_t<T> c;
  ContainerType& operator=(const ContainerType&) = delete;
};


template<typename Container>
reversed_t<Container> reversed(const Container& container) {
  return reversed_t<Container>(container);
}

inline reversed_t<objects_t> top_to_bottom(const objects_t& objects){
  return reversed_t<objects_t>(objects);
}

template<typename Container>
class but_first_t {
private:
  const ContainerType<Container> m_container;
public:
  but_first_t(const Container& c)
    : m_container(c)
  {}

  auto begin() const -> decltype(m_container.c.begin()) {
    return m_container.c.empty() ? m_container.c.end() :
      m_container.c.begin() + 1;
  }

  auto end() const -> decltype(m_container.c.end()) {
    return m_container.c.end();
  }

  using value_type = typename Container::value_type;
  but_first_t& operator=(const but_first_t&) = delete;
};

template<typename Container>
auto first(const Container& c){
  // Merely does c.front(), for consistency when used with but_first
  assert(!c.empty());
  return c.front();
}

template<typename T>
class ContainerType<but_first_t<T> >{
public:
  ContainerType(const but_first_t<T>& container)
    : c(container)
  {}

  const but_first_t<T> c;
  ContainerType& operator=(const ContainerType&) = delete;
};

template<typename Container>
but_first_t<Container> but_first(const Container& container) {
  return but_first_t<Container>(container);
}

template<typename Container>
class but_last_t {
private:
  const ContainerType<Container> m_container;
public:
  but_last_t(const Container& c)
    : m_container(c)
  {}

  auto begin() -> decltype(m_container.c.begin()) {
    return m_container.c.begin();
  }

  auto end() -> decltype(m_container.c.end()) {
    return m_container.c.empty() ?
      m_container.c.end() :
      m_container.c.begin() + resigned(m_container.c.size()) - 1;
  }

  auto rbegin() const -> decltype(m_container.c.rbegin()) {
    return m_container.c.empty() ?
      m_container.c.rend() :
      m_container.c.rbegin() + 1;
  }

  auto rend() const -> decltype(m_container.c.rend()) {
    return m_container.c.crend();
  }

  using value_type = typename Container::value_type;

private:
  but_last_t& operator=(const but_last_t&) = delete;
};

template<typename T>
class ContainerType<but_last_t<T> >{
public:
  ContainerType(const but_last_t<T>& container)
    : c(container)
  {}
  const but_last_t<T> c;
private:
  ContainerType& operator=(const ContainerType&) = delete;
};

template<typename Container>
but_last_t<Container> but_last(const Container& container) {
  return but_last_t<Container>(container);
}

template<typename T>
class enum_iter_t{
  // Iterator for enums values.
public:
  enum_iter_t(T value)
    : m_value(static_cast<T2>(value))
  {}

  T operator*() const{
    return static_cast<T>(m_value);
  }

  void operator++(){
    m_value++;
  }

  bool operator!=(const enum_iter_t& rhs) const{
    return m_value != rhs.m_value;
  }
private:
  using T2 = typename std::underlying_type<T>::type;
  T2 m_value;
};

template<typename T>
class iterable{
  // Wrapper which makes enums with BEGIN and END members iterable.
public:
  iterable(){}

  enum_iter_t<T> begin(){
    return enum_iter_t<T>(T::BEGIN);
  }
  enum_iter_t<T> end(){
    return enum_iter_t<T>(T::END);
  }
private:
};

template<typename T>
class CountedItem{
public:
  CountedItem(int num, const T& item)
    : num(num),
      item(item)
  {}

  const T& operator*() const{
    return item;
  }

  const T* operator->() const{
    return &item;
  }

  const int num;
  const T& item;
private:
  CountedItem& operator=(const CountedItem&) = delete;
};

template<typename ITER_T, typename VALUE_T>
class enumerate_iter_t{
private:
  ITER_T m_iter;
  int m_num;
public:
  enumerate_iter_t(ITER_T iter):
    m_iter(iter),
    m_num(0)
  {}

  auto operator*() -> CountedItem<VALUE_T>{
    return CountedItem<VALUE_T>(m_num, *m_iter);
  }

  enumerate_iter_t operator++(){
    ++m_iter;
    ++m_num;
    return *this;
  }

  bool operator!=(const enumerate_iter_t& other){
    return m_iter != other.m_iter;
  }
};

template<typename Container>
class enumerate_t{
private:
  const ContainerType<Container> m_container;
public:
  enumerate_t(const Container& c)
    : m_container(c)
  {}

  auto begin() const -> enumerate_iter_t<decltype(m_container.c.begin()), typename Container::value_type> {
    return enumerate_iter_t<decltype(m_container.c.begin()), typename Container::value_type>(m_container.c.begin());
  }

  auto end() const -> enumerate_iter_t<decltype(m_container.c.end()), typename Container::value_type> {
    return enumerate_iter_t<decltype(m_container.c.end()), typename Container::value_type>(m_container.c.end());
  }
private:
  enumerate_t& operator=(const enumerate_t&) = delete;
};


template<typename Container>
enumerate_t<Container> enumerate(const Container& container) {
  return enumerate_t<Container>(container);
}

template<typename T1, typename T2>
class zip_iter_t{
public:
  zip_iter_t(const T1& it1, const T2& it2)
    : m_it1(it1),
      m_it2(it2)
  {}

  zip_iter_t& operator++(){
    m_it1++;
    m_it2++;
    return *this;
  }

  bool operator!=(const zip_iter_t& other) const{
    return m_it1 != other.m_it1;
  }

  auto operator*() -> std::pair<const typename T1::value_type&,
                                const typename T2::value_type&>
  {
    const typename T1::value_type& v1(*m_it1);
    const typename T2::value_type& v2(*m_it2);
    return std::pair<const typename T1::value_type&, const typename T2::value_type&>
      (v1, v2);
  }
private:
  T1 m_it1;
  T2 m_it2;
};

template<typename T1, typename T2>
zip_iter_t<T1, T2> zip_iter(const T1& it1, const T2& it2){
  return zip_iter_t<T1, T2>(it1, it2);
}

template<typename C1, typename C2>
class zip_generator_t{
private:
  C1& m_c1;
  C2& m_c2;

public:
  zip_generator_t(C1& c1, C2& c2) :
    m_c1(c1),
    m_c2(c2)
  {
    assert(c1.size() == c2.size());
  }

  auto begin() const -> zip_iter_t<decltype(std::begin(m_c1)), decltype(std::begin(m_c2))>
  {
    return zip_iter(std::begin(m_c1), std::begin(m_c2));
  }

  auto end() const -> zip_iter_t<decltype(std::begin(m_c1)), decltype(std::begin(m_c2))>
  {
    return zip_iter(std::end(m_c1), std::end(m_c2));
  }
  zip_generator_t& operator=(const zip_generator_t&) = delete;
};

// Returns a generator which when iterated provides the elements
// pair-wise from the two containers.
template<typename C1, typename C2>
zip_generator_t<C1, C2> zip(C1& c1, C2& c2){
  return zip_generator_t<C1, C2>(c1, c2);
}

} // namespace

#endif
