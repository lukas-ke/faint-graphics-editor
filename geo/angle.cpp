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

#include <cassert>
#include <cmath>
#include "geo/angle.hh"
#include "util/math-constants.hh"

namespace faint{

void Angle::operator+=(const Angle& other){
  m_radians += other.Rad();
}

bool Angle::operator==(const Angle& other) const{
  return m_radians == other.m_radians;
}

bool Angle::operator!=(const Angle& other) const{
  return m_radians != other.m_radians;
}

Angle Angle::operator+(const Angle& other) const{
  return Angle(m_radians + other.m_radians);
}

Angle Angle::operator/(coord rhs) const{
  assert(rhs != 0);
  return Angle(m_radians / rhs);
}

Angle Angle::operator-() const{
  return Angle(-m_radians);
}

Angle Angle::operator-(const Angle& rhs) const{
  return Angle(m_radians - rhs.m_radians);
}

bool Angle::operator<(const Angle& rhs) const{
  return m_radians < rhs.m_radians;
}

bool Angle::operator<=(const Angle& rhs) const{
  return m_radians <= rhs.m_radians;
}

bool Angle::operator>(const Angle& rhs) const{
  return m_radians > rhs.m_radians;
}

bool Angle::operator>=(const Angle& rhs) const{
  return m_radians >= rhs.m_radians;
}

Angle abs(const Angle& a){
  return Angle::Rad(std::fabs(a.Rad()));
}

Angle atan2(coord y, coord x){
  return Angle::Rad(std::atan2(y,x));
}

coord sin(const Angle& a){
  return std::sin(a.Rad());
}

coord cos(const Angle& a){
  return std::cos(a.Rad());
}

coord tan(const Angle& a){
  return std::tan(a.Rad());
}

Angle normalized(const Angle& a){
  auto radians = std::fmod(a.Rad(), 2 * math::pi);
  return (radians < 0) ?
    Angle::Rad(radians + 2 * math::pi) :
    Angle::Rad(radians);
}

bool rather_zero(const Angle& angle){
  return fabs(angle.Rad()) <= coord_epsilon;
}

Angle operator*(const Angle& lhs, coord rhs){
  return Angle::Rad(lhs.Rad() * rhs);
}

Angle operator*(coord lhs, const Angle& rhs){
  return Angle::Rad(lhs * rhs.Rad());
}

} // namespace
