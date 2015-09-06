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

#ifndef FAINT_PRECISION_HH
#define FAINT_PRECISION_HH

namespace faint{

class Precision{
public:
  explicit constexpr Precision(int decimals) : m_decimals(decimals)
  {}

  Precision(float) = delete;

  operator int() const{
    return m_decimals;
  }

private:
  int m_decimals;
};

inline Precision operator "" _dec(unsigned long long decimals){
  return Precision(static_cast<int>(decimals));
}

} // namespace

#endif
