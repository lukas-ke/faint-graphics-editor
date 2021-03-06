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

#ifndef FAINT_OPTIONAL_HH
#define FAINT_OPTIONAL_HH
#include <cassert>
#include <utility>
#include "util/plain-type.hh"

namespace faint{

// The opthelper templates are helpers for customizing creation and
// destruction of the content of an \ref(Optional) depending on its type
// qualifiers.
template<typename T>
struct opthelper{
  // Value type.
  // Copied or moved into a newed pointer,
  // deleted on cleanup.
  static T* create(T&& obj){
    return new T(std::move(obj));
  }

  static T* create(T& obj){
    return new T(obj);
  }

  static T* create(const T& obj){
    return new T(obj);
  }

  static void clean_up(T*& obj){
    delete obj;
    obj = nullptr;
  }
};

template<typename T>
struct opthelper<T&>{
  // Mutable reference type, the Optional points to the external
  // object. Only instantiatable from non-const reference. Not
  // destroyed when replaced or cleared.
  static T* create(T& obj){
    return &obj;
  }

  static void clean_up(T*& obj){
    obj = nullptr;
  }
};

template<typename T>
struct opthelper<const T&>{
  // Const reference type, the Optional points to the external
  // object. Not destroyed when replaced or cleared.
  static const T* create(const T& obj){
    return &obj;
  }

  static void clean_up(const T*& obj){
    obj = nullptr;
  }
};

template<typename T>
class Optional{
  // \def(Optional)Optional;
  // Wrapper for optionally specified class members, return types and
  // parameters.
  //
  // When instantiated with the one-parameter constructor, IsSet()
  // will return true, and the Get-functions will return the
  // member. When default-constructed, the getters will instead
  // assert. This is less error prone than using pointers and not as
  // bulky as a boolean and a member.
public:
  using PT = plain_type_t<T>;

  Optional()
    : m_obj(nullptr)
  {}

  explicit Optional(PT& obj) :
    m_obj(opthelper<T>::create(obj))
  {}

  explicit Optional(const PT& obj) :
      m_obj(opthelper<T>::create(obj))
  {}

  Optional(PT&& obj) :
    m_obj(opthelper<T>::create(std::move(obj)))
  {}

  Optional(const T& obj, bool condition)
    : m_obj(nullptr)
  {
    if (condition){
      m_obj = opthelper<T>::create(obj);
    }
  }

  Optional(const Optional& other){
    if (other.m_obj == nullptr){
      m_obj = nullptr;
    }
    else{
      m_obj = opthelper<T>::create(*other.m_obj);
    }
  }

  template<typename T2=T,
           typename=std::enable_if_t<!std::is_reference_v<T2>, T2>>
  Optional(Optional&& other){
    if (other.m_obj == nullptr){
      m_obj = nullptr;
    }
    else{
      m_obj = opthelper<T>::create(std::move(*other.m_obj));
    }
  }

  ~Optional(){
    opthelper<T>::clean_up(m_obj);
  }

  Optional& operator=(const Optional& other){
    if (this == &other){
      return *this;
    }

    opthelper<T>::clean_up(m_obj);
    if (other.m_obj == nullptr){
      m_obj = nullptr;
    }
    else{
      m_obj = opthelper<T>::create(*other.m_obj);
    }
    return *this;
  }

  Optional& operator=(Optional&& other){
    if (this == &other){
      return *this;
    }

    opthelper<T>::clean_up(m_obj);
    if (other.m_obj == nullptr){
      m_obj = nullptr;
    }
    else{
      m_obj = other.m_obj;
      other.m_obj = nullptr;
    }
    return *this;
  }

  void Clear(){
    opthelper<T>::clean_up(m_obj);
  }

  T& Get(){
    assert(m_obj != nullptr);
    return *m_obj;
  }

  const T& Get() const{
    assert(m_obj != nullptr);
    return *m_obj;
  }

  T Take(){
    assert(m_obj != nullptr);
    T value(std::move(*m_obj));
    Clear();
    return value;
  }

  bool IsSet() const{
    return m_obj != nullptr;
  }

  bool NotSet() const{
    return m_obj == nullptr;
  }

  // Returns the value or the specified alternative.
  //
  // Note: Not available for Optionals holding reference types, since
  // this would either risk returning an expiring reference to the
  // alternative argument or, if returning as value type, copying a
  // held value object not meant to be copied or not supporting copy.
  template<typename T2=T>
  std::enable_if_t<!std::is_reference_v<T2>, T2> Or(const PT& alternative) const{
    if (m_obj == nullptr){
      return alternative;
    }
    return *m_obj;
  }

  void Set(const PT& obj){
    opthelper<T>::clean_up(m_obj);
    m_obj = opthelper<T>::create(obj);
  }

  void Set(PT&& obj){
    opthelper<T>::clean_up(m_obj);
    m_obj = opthelper<T>::create(std::move(obj));
  }

  template<typename FUNC>
  void IfSet(const FUNC& whenSet) const{
    if (m_obj != nullptr){
      whenSet(*m_obj);
    }
  }

  template<typename FUNC1, typename FUNC2>
  auto Visit(const FUNC1& whenSet, const FUNC2& otherwise) const{
    return m_obj != nullptr ?
      whenSet(*m_obj) : otherwise();
  }

  template<typename FUNC, typename RET>
  RET VisitSimple(const FUNC& whenSet, const RET& otherwise) const{
    return m_obj != nullptr ?
      whenSet(*m_obj) : otherwise;
  }

  explicit operator bool() const{
    return m_obj != nullptr;
  }
private:
  std::remove_reference_t<T>* m_obj;
};

template<typename T>
bool operator==(const Optional<T>& obj, const T& other){
  return obj.IsSet() && obj.Get() == other;
}

template<typename T>
bool operator==(const Optional<T>& lhs, const Optional<T>& rhs){
  return lhs.Visit(
    [&rhs](const T& v){
      return rhs == v;
    },
    [&rhs](){
      return rhs.NotSet();
    });
}

template<typename T>
bool operator!=(const Optional<T>& obj, const T& other){
  return obj.NotSet() || obj.Get() != other;
}

template<typename T>
bool operator!=(const Optional<T>& lhs, const Optional<T>& rhs){
  return !(lhs == rhs);
}

template<typename T>
Optional<T> option(const T& obj){
  return Optional<T>(obj);
}

// This class allows returning an uninitialized Optional from a
// function without having to repeat the type.
class no_option{
public:
  template<typename T>
  operator Optional<T>(){
    return Optional<T>();
  }
};

template<typename T>
bool optional_eq(const Optional<T>& o, const T& value){
  return o.IsSet() && o.Get() == value;
}

template<typename T>
T& operator|=(T& lhs, const Optional<T>& rhs){
  if (rhs.IsSet()){
    lhs = rhs.Get();
  }
  return lhs;
}

template<typename T>
Optional<T>& operator|=(Optional<T>& lhs, const Optional<T>& rhs){
  if (rhs.IsSet()){
    lhs.Set(rhs.Get());
  }
  return lhs;
}

template<typename T>
auto otherwise(T&& v){
  return [=](){return v;};
}

template<typename T, typename Exception>
T or_throw(Optional<T>&& opt, const Exception& ex){
  if (opt.NotSet()){
    throw ex;
  }
  return opt.Get();
}

} // namespace

#endif
