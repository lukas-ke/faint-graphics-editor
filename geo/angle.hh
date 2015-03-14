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

#ifndef FAINT_ANGLE_HH
#define FAINT_ANGLE_HH
#include "geo/primitive.hh"
#include "util/math-constants.hh"

namespace faint{

class Angle{
public:
  static constexpr Angle Rad(coord);
  static constexpr Angle Deg(coord);
  static constexpr Angle Zero();

  constexpr coord Deg() const{ return math::degrees_from_radians(m_radians); }
  constexpr coord Rad() const{ return m_radians; }

  void operator+=(const Angle&);
  bool operator==(const Angle&) const;
  bool operator!=(const Angle&) const;
  Angle operator+(const Angle&) const;
  Angle operator/(coord) const;
  Angle operator-() const;
  Angle operator-(const Angle&) const;
  bool operator<(const Angle&) const;
  bool operator<=(const Angle&) const;
  bool operator>(const Angle&) const;
  bool operator>=(const Angle&) const;
private:
  explicit constexpr Angle(coord radians) : m_radians(radians){};
  coord m_radians;
};

Angle operator*(const Angle&, coord);
Angle operator*(coord, const Angle&);

Angle abs(const Angle&);

// Signed angle between x-axis and x, y
Angle atan2(coord y, coord x);
coord sin(const Angle&);
coord cos(const Angle&);
coord tan(const Angle&);

Angle normalized(const Angle&);

bool rather_zero(const Angle&);

constexpr Angle Angle::Rad(coord a){
  return Angle(a);
}

constexpr Angle pi(Angle::Rad(math::pi));
constexpr Angle tau(Angle::Rad(math::tau));

constexpr Angle Angle::Deg(coord a){
  return Angle(math::radians_from_degrees(a));
}

constexpr Angle operator "" _rad(long double radians){
  return Angle::Rad(static_cast<faint::coord>(radians));
}

constexpr Angle operator "" _rad(unsigned long long radians){
  return Angle::Rad(static_cast<faint::coord>(radians));
}

constexpr Angle operator "" _deg(long double degrees){
  return Angle::Deg(static_cast<faint::coord>(degrees));
}

constexpr Angle operator "" _deg(unsigned long long degrees){
  return Angle::Deg(static_cast<faint::coord>(degrees));
}

constexpr Angle Angle::Zero(){
  return Angle(0);
}

} // namespace

#endif
