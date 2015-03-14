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

#include <fstream>
#include "util-wx/stream.hh"
#include "util-wx/file-path.hh"

namespace faint{

class BinaryReaderImpl{
public:
  BinaryReaderImpl(std::ifstream* stream)
    : stream(stream)
  {}

  ~BinaryReaderImpl(){
    delete stream;
  }
  std::ifstream* stream;
};

BinaryReader::BinaryReader(const FilePath& path){
  m_impl = new BinaryReaderImpl(
    new std::ifstream(iostream_friendly(path), std::ios::binary));
}

BinaryReader::~BinaryReader(){
  delete m_impl;
}

bool BinaryReader::eof() const{
  return m_impl->stream->eof();
}

bool BinaryReader::good() const{
  return m_impl->stream->good();
}

void BinaryReader::read(char* buffer, std::streamsize sz){
  m_impl->stream->read(buffer, sz);
}

void BinaryReader::seekg(std::streampos pos) const{
  m_impl->stream->seekg(pos);
}

std::streampos BinaryReader::tellg() const{
  return m_impl->stream->tellg();
}

BinaryReader& BinaryReader::ignore(std::streamsize n){
  m_impl->stream->ignore(n);
  return *this;
}

class BinaryWriterImpl{
public:
  BinaryWriterImpl(std::ofstream* stream)
    : stream(stream)
  {}

  ~BinaryWriterImpl(){
    delete stream;
  }
  std::ofstream* stream;
};

BinaryWriter::BinaryWriter(const FilePath& path){
  m_impl = new BinaryWriterImpl(
    new std::ofstream(iostream_friendly(path), std::ios::binary));
}

BinaryWriter::~BinaryWriter(){
  delete m_impl;
}

bool BinaryWriter::eof() const{
  return m_impl->stream->eof();
}

bool BinaryWriter::good() const{
  return m_impl->stream->good();
}

void BinaryWriter::write(const char* buffer, std::streamsize sz){
  m_impl->stream->write(buffer, sz);
}

void BinaryWriter::put(char c){
  m_impl->stream->put(c);
}

} // namespace
