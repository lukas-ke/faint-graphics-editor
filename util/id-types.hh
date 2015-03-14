// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_ID_TYPES_HH
#define FAINT_ID_TYPES_HH

namespace faint{

template<int group>
class FaintID{
public:
  FaintID(){
    static int max_id = 1007;
    m_rawId = max_id++;
  }

  static FaintID Invalid(){
    return FaintID(-1);
  }

  static FaintID DefaultID(){
    return FaintID<group>(0);
  }

  bool operator==(const FaintID<group>& other) const{
    return other.m_rawId == m_rawId;
  }

  bool operator!=(const FaintID<group>& other) const{
    return !operator==(other);
  }

  bool operator<(const FaintID<group>& other) const{
    return m_rawId < other.m_rawId;
  }

  bool operator>(const FaintID<group>& other) const{
    return m_rawId > other.m_rawId;
  }

  int Raw() const{
    return m_rawId;
  }
private:
  FaintID(int id){
    m_rawId = id;
  }
  int m_rawId;
};

using CanvasId = FaintID<107>;
using ObjectId = FaintID<108>;
using CommandId = FaintID<109>;
// 110 reserved
using FrameId = FaintID<111>;

}

#endif
