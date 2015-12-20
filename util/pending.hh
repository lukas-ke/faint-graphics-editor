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

#ifndef FAINT_PENDING_HH
#define FAINT_PENDING_HH
#include <cassert>
#include <memory>

namespace faint{

template <typename T>
class Pending {
  // Holds an item until it is taken or replaced. On destruction
  // (or replacement with Set) an untaken item will be deleted.
  //
  // Kind of silly now that I use unique_ptr, but perhaps a bit more
  // explicit.
public:
  Pending() : m_item(nullptr) {}

  void Set(T* item){
     m_item.reset(item);
  }

  void Set(std::unique_ptr<T>&& item){
    m_item = std::move(item);
  }

  std::unique_ptr<T> Take(){
    return std::unique_ptr<T>(std::move(m_item));
  }

  bool Valid() const{
    return m_item != nullptr;
  }

  Pending& operator=(const Pending&) = delete;
  Pending(const Pending&) = delete;
private:
  std::unique_ptr<T> m_item;
};

} // namespace

#endif
