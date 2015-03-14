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

#include "geo/int-size.hh"
#include "geo/padding.hh"

namespace faint{

Padding::Padding(int left, int right, int top, int bottom) :
  bottom(bottom),
  left(left),
  right(right),
  top(top)
{}

Padding Padding::All(int p){
  return Padding(p, p, p, p);
}

Padding Padding::Divide(int p){
  int h = p / 2;
  return Padding(h, h, h, h);
}

Padding Padding::Right(int p){
  return Padding(0,p,0,0);
}

Padding Padding::Bottom(int p){
  return Padding(0,0,0,p);
}

Padding Padding::None(){
  return Padding(0,0,0,0);
}

IntSize Padding::GetSize() const{
  return IntSize(left + right, bottom + top);
}

Padding Padding::operator+(const Padding& other) const{
  return Padding(left + other.left,
    right + other.right,
    top + other.top,
    bottom + other.bottom);
}

} // namespace
