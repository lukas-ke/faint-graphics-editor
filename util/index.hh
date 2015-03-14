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

#ifndef FAINT_INDEX_HH
#define FAINT_INDEX_HH
#include "geo/primitive.hh"
#include "util/distinct.hh"

namespace faint{

class Index{
public:
  constexpr Index() : m_value(0) {}
  explicit constexpr Index (int v) : m_value(v) {}
  explicit Index(size_t);
  int Get() const;
  Index& operator=(const Index& rhs);
private:
  int m_value;
};

bool operator==(const Index&, const Index&);
bool operator==(const Index&, const size_t&);
bool operator<(const Index&, const Index&);
bool operator>(const Index&, size_t);
bool operator<=(const Index&, const Index&);
bool operator!=(const Index&, const Index&);
bool operator!=(const Index&, size_t);
Index operator-(const Index&, const Index&);
Index operator-(const Index&, size_t);
bool operator<=(const Index&, size_t);
bool operator<(const Index&, size_t);

using NewIndex = Order<Index>::New;
using OldIndex = Order<Index>::Old;

size_t to_size_t(const Index&);
Index to_index(size_t);

template<typename T>
bool valid_index(const Index& index, const T& container){
  return to_size_t(index.Get()) <= container.size();
}

constexpr Index operator "" _idx(unsigned long long i){
  return Index(static_cast<int>(i));
}

} // namespace

#endif
