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

#ifndef FAINT_STREAM_HH
#define FAINT_STREAM_HH
#include <array>
#include <iosfwd> // For streampos, streamsize

namespace faint{

class FilePath;

class BinaryReaderImpl;
class BinaryReader{
public:
  BinaryReader(const FilePath&);
  ~BinaryReader();
  void read(char*, std::streamsize);
  bool eof() const;
  bool good() const;
  void seekg(std::streampos) const;
  std::streampos tellg() const;
  BinaryReader& ignore(std::streamsize);

  BinaryReader(const BinaryReader&) = delete;
  BinaryReader& operator=(const BinaryReader&) = delete;
private:
  BinaryReaderImpl* m_impl;
};

class BinaryWriterImpl;
class BinaryWriter{
public:
  BinaryWriter(const FilePath&);
  ~BinaryWriter();
  bool eof() const;
  bool good() const;
  void write(const char*, std::streamsize);
  void put(char);

  BinaryWriter(const BinaryWriter&) = delete;
  BinaryWriter& operator=(const BinaryWriter&) = delete;
private:
  BinaryWriterImpl* m_impl;
};

template<std::size_t N>
void write(BinaryWriter& out, const std::array<unsigned char, N>& a){
  out.write((const char*)a.data(), a.size());
}

template<size_t N>
std::array<unsigned char, N> read_array(BinaryReader& f){
  std::array<unsigned char, N> a;
  f.read(reinterpret_cast<char*>(a.data()), N);
  return a;
}

} // namespace

#endif
