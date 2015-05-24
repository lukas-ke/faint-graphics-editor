// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_DEFAULT_CONSTRUCTIBLE_HH
#define FAINT_DEFAULT_CONSTRUCTIBLE_HH
#include <memory>
#include <type_traits> // for assertions

namespace faint{

// Wrapper for non-default-constructible types as a place-holder
// during parsing and such.
template<typename T>
class DefaultConstructible{
public:
  static_assert(!std::is_default_constructible<T>::value,
    "T already default constructible.");

  DefaultConstructible(){}

  void Set(T&& t){
    m_obj = std::make_unique<T>(std::move(t));
  }

  operator const T&() const{
    assert(m_obj != nullptr);
    return *m_obj;
  }

  DefaultConstructible(const DefaultConstructible& other){
    if (other.m_obj != nullptr){
      m_obj = std::make_unique<T>(*other.m_obj);
    }
  }

  DefaultConstructible& operator=(const DefaultConstructible&) = delete;
private:
  std::unique_ptr<T> m_obj;
};

} // namespace

#endif
