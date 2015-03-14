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

#ifndef FAINT_SERIALIZE_TUPLE_HH
#define FAINT_SERIALIZE_TUPLE_HH
#include <array>
#include <tuple>

namespace faint {

template<typename T>
inline typename std::underlying_type<T>::type& raw_enum_value(T& e){
  return reinterpret_cast<typename std::underlying_type<T>::type&>(e);
}

template<typename T>
inline const typename std::underlying_type<T>::type& raw_enum_value(const T& e){
  return reinterpret_cast<const typename std::underlying_type<T>::type&>(e);
}

template<size_t N, class Head, class... Tail>
struct sizeof_rest{
  static const size_t value =
    sizeof(typename std::tuple_element<N, std::tuple<Head, Tail...> >::type) +
    sizeof_rest<N - 1, Head, Tail...>::value;
};

template<class Head, class... Tail>
struct sizeof_rest<0, Head, Tail...>{
  static const size_t value = sizeof(typename std::tuple_element<0,
    std::tuple<Head, Tail...>>::type);
};

template<class Head, class... Tail>
struct sizeof_entries{
  static const size_t value =
    sizeof_rest<std::tuple_size< std::tuple<Head, Tail...> >::value - 1,
      Head, Tail...>::value;
};

inline void write(unsigned char* data, const uint32_t v){
  data[3] = static_cast<unsigned char>(((v >> 24) & 0xffu));
  data[2] = static_cast<unsigned char>(((v >> 16) & 0xffu));
  data[1] = static_cast<unsigned char>(((v >> 8) & 0xffu));
  data[0] = static_cast<unsigned char>((v & 0xffu));
}

inline void write(unsigned char* data, const int32_t v){
  write(data, static_cast<uint32_t>(v));
}

inline void write(unsigned char* data, const uint16_t v){
  data[1] = static_cast<unsigned char>((v >> 8) & 0xff);
  data[0] = static_cast<unsigned char>(v & 0xff);
}

inline void write(unsigned char* data, const uint8_t v){
  data[0] = v;
}

template<size_t N, class Head, class... Tail>
struct serializer{
  template<typename ARRAY_TYPE>
  static void add_rest(const std::tuple<Head, Tail...>& t,
    ARRAY_TYPE& a,
    size_t byteOffset){
    size_t entrySize = sizeof_entries<Head, Tail...>::value;
    auto item = std::get<N - 1>(t);
    unsigned char* END = a.data() + entrySize;
    write(END - byteOffset - sizeof(item), item);
    serializer<N - 1, Head, Tail...>::add_rest(t, a, byteOffset + sizeof(item));
  }
};

template<class Head, class... Tail>
struct serializer<0, Head, Tail...>{
  template<typename ARRAY_TYPE>
  static void add_rest(const std::tuple<Head, Tail...>&, ARRAY_TYPE&, size_t){}
};

template<class Head, class... Tail>
auto serialize_tuple(const std::tuple<Head, Tail...>& t){
  std::array<unsigned char, sizeof_entries<Head, Tail...>::value> a;
  static const size_t N = std::tuple_size<std::tuple<Head, Tail...> >::value;
  serializer<N, Head, Tail...>::add_rest(t, a, 0);
  return a;
}

template<typename Writer, class Head, class... Tail>
void write_tuple(Writer& out, const std::tuple<Head, Tail...>& obj){
  auto m = serialize_tuple(obj);
  out.write(m.data(), m.size());
}

} // namespace

#endif
