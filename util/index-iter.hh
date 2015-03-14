// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_INDEX_ITER_HH
#define FAINT_INDEX_ITER_HH
#include <iterator>
#include "util/index.hh"

namespace faint{

class IndexIter{
public:
  using value_type = Index;
  using iterator_category = std::input_iterator_tag;
  using difference_type = size_t;
  using pointer = Index*;
  using reference = Index&;

  IndexIter(const Index& index)
    : index(index)
  {}

  const Index* operator->() const{
    return &index;
  }

  Index operator*() const{
    return index;
  }

  bool operator!=(const IndexIter& other) const{
    return index != other.index;
  }

  bool operator==(const IndexIter& other) const{
    return index == other.index;
  }

  IndexIter& operator++(){
    index = Index(index.Get() + 1);
    return *this;
  }

  Index index;
};

class up_to{
  // Generator for iterating [0..end)
  // Note: Excludes the end-index.
public:
  using value_type = IndexIter::value_type;
  explicit up_to(const Index& end)
    : m_end(IndexIter(end))
  {}

  explicit up_to(size_t end)
    : m_end(IndexIter(Index(end)))
  {}


  auto begin() const{
    return IndexIter(0_idx);
  }

  auto end() const{
    return m_end;
  }

  size_t size() const {
    return to_size_t(m_end->Get());
  }

  IndexIter m_end;
};

inline auto begin(const up_to& o){
  return o.begin();
}

inline auto end(const up_to& o){
  return o.end();
}

} // namespace

#endif
