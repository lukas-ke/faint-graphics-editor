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

#include <tuple>
#include "bitmap/gradient.hh"
#include "util/either.hh"
#include "util/index.hh"

namespace faint{

template<typename T>
bool point_less(const T& lhs, const T& rhs){
  return lhs.x < rhs.x || (lhs.x == rhs.x && lhs.y < rhs.y);
}

static bool operator<(const Point& lhs, const Point& rhs){
  return point_less(lhs, rhs);
}

static bool operator<(const Radii& lhs, const Radii& rhs){
  return point_less(lhs, rhs);
}

ColorStop::ColorStop()
  : m_color(0,0,0),
    m_offset(0.0)
{}

ColorStop::ColorStop(const Color& c, double offset)
  : m_color(c),
    m_offset(offset)
{}

Color ColorStop::GetColor() const{
  return m_color;
}

double ColorStop::GetOffset() const{
  return m_offset;
}

bool ColorStop::operator<(const ColorStop& other) const{
  return m_offset < other.m_offset ||
    (m_offset == other.m_offset && m_color < other.m_color);
}

bool ColorStop::operator==(const ColorStop& other) const{
  return m_color == other.m_color &&
    m_offset == other.m_offset;
}

bool ColorStop::operator!=(const ColorStop& other) const{
  return m_color != other.m_color ||
    m_offset != other.m_offset;
}

LinearGradient::LinearGradient()
  : m_angle(Angle::Zero()),
    m_objectAligned(true)
{}

LinearGradient::LinearGradient(const Angle& angle, const color_stops_t& stops)
  : m_angle(angle),
    m_objectAligned(true),
    m_stops(stops)
{}

void LinearGradient::Add(const ColorStop& stop){
  m_stops.push_back(stop);
}

void LinearGradient::Remove(const Index& index){
  assert(valid_index(index, m_stops));
  m_stops.erase(begin(m_stops) + index.Get());
}

Angle LinearGradient::GetAngle() const{
  return m_angle;
}

Index LinearGradient::GetNumStops() const{
  return Index(resigned(m_stops.size()));
}

bool LinearGradient::GetObjectAligned() const{
  return m_objectAligned;
}

const ColorStop& LinearGradient::GetStop(const Index& index) const{
  assert(valid_index(index, m_stops));
  return m_stops[to_size_t(index)];
}

const color_stops_t& LinearGradient::GetStops() const{
  return m_stops;
}

void LinearGradient::SetAngle(const Angle& a){
  m_angle = a;
}

void LinearGradient::SetObjectAligned(bool objectAligned){
  m_objectAligned = objectAligned;
}

void LinearGradient::SetStop(const Index& index, const ColorStop& stop){
  assert(valid_index(index, m_stops));
  m_stops[to_size_t(index)] = stop;
}

void LinearGradient::SetStops(const color_stops_t& stops){
  m_stops = stops;
}

inline std::tuple<Angle, bool, const color_stops_t&> tupeled(
  const LinearGradient& g)
{
  return std::make_tuple(g.GetAngle(),
    g.GetObjectAligned(),
    std::cref(g.GetStops()));
}

bool LinearGradient::operator<(const LinearGradient& other) const{
  return tupeled(*this) < tupeled(other);
}

bool LinearGradient::operator==(const LinearGradient& other) const{
  return tupeled(*this) == tupeled(other);
}

bool LinearGradient::operator!=(const LinearGradient& other) const{
  return !(*this == other);
}

bool LinearGradient::operator>(const LinearGradient& other) const{
  return tupeled(*this) > tupeled(other);
}

LinearGradient unrotated(const LinearGradient& g){
  LinearGradient g2(Angle::Zero(), g.GetStops());
  g2.SetObjectAligned(g.GetObjectAligned());
  return g2;
}

LinearGradient with_angle(const LinearGradient& g, const Angle& angle){
  LinearGradient g2(g);
  g2.SetAngle(angle);
  return g2;
}

inline std::tuple<Point, bool, Radii, const color_stops_t&> tupeled(
  const RadialGradient& g)
{
  return std::make_tuple(g.GetCenter(),
    g.GetObjectAligned(),
    g.GetRadii(),
    std::cref(g.GetStops()));
}

RadialGradient::RadialGradient()
  : m_center(0,0),
    m_objectAligned(true),
    m_radii(0,0)
{}

RadialGradient::RadialGradient(const Point& center,
  const Radii& radii,
  const color_stops_t& stops)
  : m_center(center),
    m_objectAligned(true),
    m_radii(radii),
    m_stops(stops)
{}

void RadialGradient::Add(const ColorStop& stop){
  m_stops.push_back(stop);
}

Point RadialGradient::GetCenter() const{
  return m_center;
}

Index RadialGradient::GetNumStops() const{
  return Index(resigned(m_stops.size()));
}

bool RadialGradient::GetObjectAligned() const{
  return m_objectAligned;
}

Radii RadialGradient::GetRadii() const{
  return m_radii;
}

ColorStop RadialGradient::GetStop(const Index& index) const{
  assert(valid_index(index, m_stops));
  return m_stops[to_size_t(index)];
}

const color_stops_t& RadialGradient::GetStops() const{
  return m_stops;
}

void RadialGradient::Remove(const Index& index){
  assert(valid_index(index, m_stops));
  m_stops.erase(begin(m_stops) + index.Get());
}

void RadialGradient::SetCenter(const Point& center){
  m_center = center;
}

void RadialGradient::SetObjectAligned(bool value){
  m_objectAligned = value;
}

void RadialGradient::SetRadii(const Radii& radii){
  m_radii = radii;
}

void RadialGradient::SetStop(const Index& index, const ColorStop& stop){
  assert(valid_index(index, m_stops));
  m_stops[to_size_t(index)] = stop;
}

void RadialGradient::SetStops(const color_stops_t& stops){
  m_stops = stops;
}

bool RadialGradient::operator==(const RadialGradient& other) const{
  return tupeled(*this) == tupeled(other);
}

bool RadialGradient::operator!=(const RadialGradient& other) const{
  return !(*this == other);
}

bool RadialGradient::operator<(const RadialGradient& other) const{
  return tupeled(*this) < tupeled(other);
}

bool RadialGradient::operator>(const RadialGradient& other) const{
  return tupeled(*this) > tupeled(other);
}

class Gradient::GradientImpl{
public:
  GradientImpl(const LinearGradient& linear)
    : gradient(linear)
  {}

  GradientImpl(const RadialGradient& radial)
    : gradient(radial)
  {}

  Either<LinearGradient, RadialGradient> gradient;
};

Gradient::Gradient(const LinearGradient& linear)
  : m_impl(std::make_unique<GradientImpl>(linear))
{}

Gradient::Gradient(const RadialGradient& radial)
  : m_impl(std::make_unique<GradientImpl>(radial))
{}

Gradient::Gradient(const Gradient& other)
  : m_impl(std::make_unique<GradientImpl>(*other.m_impl))
{}

Gradient::~Gradient(){
  // This destructor must be defined or unique_ptr to forward-declared
  // GradientImpl won't compile.
}

color_stops_t Gradient::GetStops() const{
  return m_impl->gradient.Visit(
    [](const LinearGradient& g){
      return g.GetStops();
    },
    [](const RadialGradient& g){
      return g.GetStops();
    });
}

bool Gradient::IsLinear() const{
  return m_impl->gradient.Get<LinearGradient>().IsSet();
}

bool Gradient::IsRadial() const{
  return m_impl->gradient.Get<RadialGradient>().IsSet();
}

bool Gradient::operator==(const Gradient& other) const{
  return m_impl->gradient == other.m_impl->gradient;
}

const LinearGradient& Gradient::GetLinear() const{
  return m_impl->gradient.Expect<LinearGradient>();
}

const RadialGradient& Gradient::GetRadial() const{
  return m_impl->gradient.Expect<RadialGradient>();
}

} // namespace
