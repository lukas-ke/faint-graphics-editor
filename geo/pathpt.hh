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

#ifndef FAINT_PATHPT_HH
#define FAINT_PATHPT_HH
#include <cassert>
#include "geo/angle.hh"
#include "geo/geo-fwd.hh"
#include "geo/point.hh"
#include "geo/radii.hh"

namespace faint{

class ArcTo{
public:
  ArcTo(const Radii& r,
    const Angle& axisRotation,
    int largeArcFlag,
    int sweepFlag,
    const Point& p)
    : axisRotation(axisRotation),
      largeArcFlag(largeArcFlag),
      p(p),
      r(r),
      sweepFlag(sweepFlag)
  {}
  Angle axisRotation;
  int largeArcFlag;
  Point p;
  Radii r;
  int sweepFlag;
};

class Close{};

class CubicBezier{
  // A cubic bezier end point (p) and the two control points (c, d)
public:
  CubicBezier(const Point& p, const Point& c, const Point& d)
    : c(c),
      d(d),
      p(p)
  {}
  Point c;
  Point d;
  Point p;
};

class LineTo{
public:
  LineTo(const Point& p)
    : p(p)
  {}
  Point p;
};

class MoveTo{
public:
  MoveTo(const Point& p)
    : p(p)
  {}
  Point p;
};

class PathPt {
public:
  enum class Type {
    ArcTo,
    Close,
    CubicBezier,
    LineTo,
    MoveTo
  };

  Type type;
  Point p;
  Point c;
  Point d;
  Radii r;
  Angle axisRotation;
  int largeArcFlag;
  int sweepFlag;

  static PathPt PathCloser(){
    return PathPt(Type::Close);
  }

  static PathPt MoveTo(const Point& p){
    return PathPt(Type::MoveTo, p);
  }

  static PathPt LineTo(const Point& p){
    return PathPt(Type::LineTo, p);
  }

  static PathPt CubicBezierTo(const Point& p, const Point& c, const Point& d){
    return PathPt(Type::CubicBezier, p, c, d);
  }

  static PathPt Arc(const Radii& r,
    const Angle& axisRotation,
    int largeArcFlag,
    int sweepFlag,
    const Point& p)
  {
    PathPt pt(Type::ArcTo);
    pt.p = p;
    pt.c.x = 0.0;
    pt.c.y = 0.0;
    pt.d.x = 0.0;
    pt.d.y = 0.0;
    pt.r = r;
    pt.axisRotation = axisRotation;
    pt.largeArcFlag = largeArcFlag;
    pt.sweepFlag = sweepFlag;
    return pt;
  }

  bool ClosesPath() const{
    return type == Type::Close;
  }

  bool IsArc() const{
    return type == Type::ArcTo;
  }

  bool IsCubicBezier() const{
    return type == Type::CubicBezier;
  }

  bool IsLine() const{
    return type == Type::LineTo;
  }

  bool IsMove() const{
    return type == Type::MoveTo;
  }

  bool IsNotMove() const{
    return !IsMove();
  }

  PathPt Rotated(const Angle&, const Point& pivot) const;

  template<typename ARCF,
           typename CLOSEF,
           typename CUBICF,
           typename LINEF,
           typename MOVEF>
  auto Visit(const ARCF& arcFunc,
    const CLOSEF& closeFunc,
    const CUBICF& cubicFunc,
    const LINEF& lineFunc,
    const MOVEF& moveFunc) const -> decltype(closeFunc(faint::Close()))
  {
    switch (type){
    case Type::ArcTo:
      return arcFunc(faint::ArcTo(r, axisRotation, largeArcFlag, sweepFlag, p));

    case Type::Close:
      return closeFunc(faint::Close());

    case Type::CubicBezier:
      return cubicFunc(faint::CubicBezier(p, c, d));

    case Type::LineTo:
      return lineFunc(faint::LineTo(p));

    case Type::MoveTo:
      return moveFunc(faint::MoveTo(p));
    }

    assert(false);
    return closeFunc(faint::Close());
  }

private:
  PathPt(Type t)
    : type(t),
      axisRotation(Angle::Zero()),
      largeArcFlag(0),
      sweepFlag(0)
  {}

  PathPt(Type t, const Point& p)
    : type(t),
      p(p),
      axisRotation(Angle::Zero()),
      largeArcFlag(0),
      sweepFlag(0)
  {}

  PathPt(Type t, const Point& p, const Point& c, const Point& d) :
    type(t),
    p(p),
    c(c),
    d(d),
    axisRotation(Angle::Zero()),
    largeArcFlag(0),
    sweepFlag(0)
  {}
};

} // namespace

#endif
