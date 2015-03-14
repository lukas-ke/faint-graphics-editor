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

#ifndef FAINT_AT_MOST_HH
#define FAINT_AT_MOST_HH
#include <cassert>
#include <vector>

namespace faint{

template<typename T>
class AtMost2{
public:
  AtMost2(){}
  AtMost2(const T& v){
    m_items.emplace_back(v);
  }

  AtMost2(const T& v0, const T& v1){
    m_items.emplace_back(v0);
    m_items.emplace_back(v1);
  }

  template<typename FUNC0, typename FUNC1, typename FUNC2>
  auto Visit(const FUNC0& func0, const FUNC1& func1, const FUNC2& func2)
    -> decltype(func0())
  {
    size_t count = m_items.size();
    if (count == 0){
      return func0();
    }
    else if (count == 1){
      return func1(m_items[0]);
    }
    else{
      assert(count == 2);
      return func2(m_items[0], m_items[1]);
    }
  }
private:
  std::vector<T> m_items;
};

} // namespace

#endif
