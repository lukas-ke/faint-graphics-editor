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

#ifndef FAINT_FREEZER_HH
#define FAINT_FREEZER_HH

namespace faint{

template<typename T>
class Freezer{
  // Scoped freezing of wxWidgets windows.
public:
  Freezer(T* w)
    : m_window(w)
  {
    m_window->Freeze();
  }

  Freezer(Freezer&& other){
    m_window = other.m_window;
    other.m_window = nullptr;
  }

  ~Freezer(){
    if (m_window != nullptr){
      m_window->Thaw();
    }
  }

private:
  T* m_window;
};

template<typename T>
Freezer<T> freeze(T* window){
  return Freezer<T>(window);
}

template<typename T>
Freezer<typename T::element_type> freeze(T& window){
  return Freezer<typename T::element_type>(window.get());
}

} // namespace

#endif
