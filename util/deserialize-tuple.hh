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

#ifndef FAINT_DESERIALIZE_TUPLE_HH
#define FAINT_DESERIALIZE_TUPLE_HH
#include "util/serialize-tuple.hh"
#include <array>
#include <tuple>

namespace faint{

inline void read(const unsigned char* data, uint32_t& v){
  v = (static_cast<uint32_t>(data[3]) << 24) |
    (static_cast<uint32_t>(data[2]) << 16) |
    (static_cast<uint32_t>(data[1]) << 8) |
    static_cast<uint32_t>(data[0]);
}

inline void read(const unsigned char* data, uint8_t& v){
  v = static_cast<uint8_t>(data[0]);
}

inline void read(const unsigned char* data, int32_t& v){
  v = static_cast<int32_t>((static_cast<uint32_t>(data[3]) << 24) |
    (static_cast<uint32_t>(data[2]) << 16) |
    (static_cast<uint32_t>(data[1]) << 8) |
    static_cast<uint32_t>(data[0]));
}

inline void read(const unsigned char* data, uint16_t& v){
  v = static_cast<uint16_t>((static_cast<uint16_t>(data[1]) << 8) |
    (static_cast<uint16_t>(data[0])));
}

template<size_t N, class Head, class... Tail>
struct deserializer{
  template<typename ARRAY_TYPE>
  static void add_rest(std::tuple<Head, Tail...>& t,
    const ARRAY_TYPE& a,
    size_t byteOffset)
  {
    size_t entrySize = sizeof_entries<Head, Tail...>::value;
    auto& item = std::get<N - 1>(t);
    const unsigned char* END = a.data() + entrySize;
    read(END - byteOffset - sizeof(item), item);
    deserializer<N - 1, Head, Tail...>::add_rest(t, a, byteOffset +
      sizeof(item));
  }
};

template<class Head, class... Tail>
struct deserializer<0, Head, Tail...>{
  template<typename ARRAY_TYPE>
  static void add_rest(std::tuple<Head, Tail...>&,
    const ARRAY_TYPE&,
    size_t)
  {}
};

template<class Head, class... Tail>
void deserialize_tuple(std::tuple<Head, Tail...>& t,
  const std::array<unsigned char, sizeof_entries<Head, Tail...>::value>& a)
{
  static const size_t N = std::tuple_size<std::tuple<Head, Tail...> >::value;
  deserializer<N, Head, Tail...>::add_rest(t, a, 0);
}

template<typename TUPLE, typename ARRAY>TUPLE deserialize_tuple2(const ARRAY& a){
  TUPLE t;
  deserialize_tuple(t, a);
  return t;
}

} // namespace

#endif
