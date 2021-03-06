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
  Point c; // First control point
  Point d; // Second control point
  Point p; // End-point
};

class LineTo{
public:
  explicit LineTo(const Point& p) : p(p) {}
  Point p;
};

class MoveTo{
public:
  explicit MoveTo(const Point& p) : p(p) {}
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
    return {Type::MoveTo, p};
  }

  static PathPt LineTo(const Point& p){
    return {Type::LineTo, p};
  }

  PathPt(const faint::ArcTo& arcTo)
    : type(Type::ArcTo),
      p(arcTo.p),
      r(arcTo.r),
      axisRotation(arcTo.axisRotation),
      largeArcFlag(arcTo.largeArcFlag),
      sweepFlag(arcTo.sweepFlag)
  {}

  PathPt(const CubicBezier& bezier)
    : type(Type::CubicBezier),
      p(bezier.p),
      c(bezier.c),
      d(bezier.d),
      axisRotation(Angle::Zero()),
      largeArcFlag(0),
      sweepFlag(0)
  {}

  PathPt(const faint::Close&)
    : type(Type::Close),
      axisRotation(Angle::Zero())
  {}

  PathPt(const faint::LineTo& line)
    : type(Type::LineTo),
      p(line.p),
      axisRotation(Angle::Zero()),
      largeArcFlag(0),
      sweepFlag(0)
  {}

  PathPt(const faint::MoveTo& move)
    : type(Type::MoveTo),
      p(move.p),
      axisRotation(Angle::Zero())
  {}

  static PathPt CubicBezierTo(const Point& p, const Point& c, const Point& d){
    return {Type::CubicBezier, p, c, d};
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

  template<typename CubicBezierFunc>
  void IfCubicBezier(const CubicBezierFunc& f) const{
    if (type == Type::CubicBezier){
      f(faint::CubicBezier(p, c, d));
    }
  }

  bool operator==(const PathPt& other) const{
    if (type != other.type){
      return false;
    }

    switch (type){
    case Type::ArcTo:
      return p == other.p &&
        r == other.r &&
        axisRotation == other.axisRotation &&
        largeArcFlag == other.largeArcFlag &&
        sweepFlag == other.sweepFlag;

    case Type::Close:
      return true;

    case Type::CubicBezier:
      return p == other.p &&
        c == other.c &&
        d == other.d;

    case Type::LineTo:
      return p == other.p;

    case Type::MoveTo:
      return p == other.p;
    }

    assert(false);
    return false;
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
