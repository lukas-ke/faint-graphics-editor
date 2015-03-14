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

#ifndef FAINT_SERIALIZE_BMP_TYPES_HH
#define FAINT_SERIALIZE_BMP_TYPES_HH
#include "formats/bmp/bmp-types.hh"
#include "formats/bmp/bmp-errors.hh"
#include "util/optional.hh"
#include "util/serialize-tuple.hh"
#include "util-wx/stream.hh"

namespace faint{

class ReadBmpError{
public:
  ReadBmpError(const utf8_string& message)
    : message(message)
  {}

  utf8_string message;

  ReadBmpError operator=(const ReadBmpError&) = delete;
};

template<size_t N>
using Buffer = std::array<unsigned char, N>;

template<typename T>
struct StructInfo{};

template<>
struct StructInfo<BitmapFileHeader>{
  static const size_t bytes = 14;
  static const char* StructName(){return "BITMAPFILEHEADER";}
};

template<>
struct StructInfo<BitmapInfoHeader>{
  static const size_t bytes = 40;
  static const char* StructName(){return "BITMAPINFOHEADER";}
};

template<>
struct StructInfo<IconDir>{
  static const size_t bytes = 6;
  static const char* StructName(){return "ICONDIR";}
};

template<>
struct StructInfo<IconDirEntry>{
  static const size_t bytes = 16;
  static const char* StructName(){return "ICONDIRENTRY";}
};

template<class HEAD, class... Tail>
struct struct_lengths_t{
  static const size_t value = StructInfo<HEAD>::bytes + struct_lengths_t<Tail...>::value;
};

template<typename HEAD>
struct struct_lengths_t<HEAD>{
  static const size_t value = StructInfo<HEAD>::bytes;
};

template<class HEAD, class... Tail>
constexpr size_t struct_lengths(){
  return struct_lengths_t<HEAD, Tail...>::value;
}

inline auto tied(BitmapFileHeader& h){
  return std::tie(h.fileType,
    h.fileLength,
    h.reserved1,
    h.reserved2,
    h.dataOffset);
}

inline auto tied(BitmapInfoHeader& h){
  return std::tie(h.headerLen,
    h.width,
    h.height,
    h.colorPlanes,
    h.bpp,
    raw_enum_value(h.compression),
    h.rawDataSize,
    h.horizontalResolution,
    h.verticalResolution,
    h.paletteColors,
    h.importantColors);
}

inline auto tied(IconDir& h){
  return std::tie(h.reserved,
    raw_enum_value(h.imageType),
    h.imageCount);
}

inline auto tied(IconDirEntry& h){
  return std::tie(h.width,
    h.height,
    h.reserved,
    h.colorCount,
    h.colorPlanes,
    h.bpp,
    h.bytes,
    h.offset);
}

template<typename T>
auto serialize_struct(const T& h){
  T copy = h;
  return serialize_tuple(tied(copy));
}

template<typename T>
T deserialize_struct(const Buffer<StructInfo<T>::bytes>& a){
  T h;
  auto t(tied(h));
  deserialize_tuple(t, a);
  return h;}

template<typename T>
void write_struct(BinaryWriter& out, const T& h){
  auto a = serialize_struct(h);
  write(out, a);
}

template<typename T>
Optional<T> read_struct(BinaryReader& in){
  auto a = read_array<StructInfo<T>::bytes>(in);
  if (!in.good()){
    return {};
  }
  return deserialize_struct<T>(a);
}

inline bool invalid_header_length(int len){
  return len != StructInfo<BitmapInfoHeader>::bytes;
}

template<typename T>
T or_throw(const Optional<T>& o, const char* s){
  return o.Visit(
    [](const T& o) -> T{
      return o;
    },
    [&]() -> T{
      throw ReadBmpError(s);
    });
}

template<typename T, typename FUNC>
T or_throw(const Optional<T>& o, const FUNC& f){
  return o.Visit(
    [](const T& v) -> T{
      return v;
    },
    [&]() -> T{
      throw ReadBmpError(f());
    });
}

template<typename T>
T read_struct_or_throw(BinaryReader& in){
  auto o = read_struct<T>(in);
  return or_throw(o,
    [](){
      return error_premature_eof(StructInfo<T>::StructName());
    });
}

} // namespace

#endif
