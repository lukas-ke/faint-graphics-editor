// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_HANDLE_FUNC_HH
#define FAINT_HANDLE_FUNC_HH
#include "geo/line.hh"
#include "geo/object-handle.hh"
#include "geo/point.hh"
#include "geo/rect.hh"
#include "geo/tri.hh"

namespace faint{

template<Handle H>
struct handle_return{};
template<> struct handle_return<Handle::P0>{using type = Point;};
template<> struct handle_return<Handle::P1>{using type = Point;};
template<> struct handle_return<Handle::P2>{using type = Point;};
template<> struct handle_return<Handle::P3>{using type = Point;};
template<> struct handle_return<Handle::P0P1>{using type =  LineSegment;};
template<> struct handle_return<Handle::P0P2>{using type = LineSegment;};
template<> struct handle_return<Handle::P1P3>{using type = LineSegment;};
template<> struct handle_return<Handle::P2P3>{using type = LineSegment;};

// Gets the Point from the Tri for the specified handle for corner handles,
// or a line between the corners for side handles.
template<Handle H>
typename handle_return<H>::type get(const Tri&){}

template<>
inline Point get<Handle::P0>(const Tri& tri){
  return tri.P0();
}

template<>
inline Point get<Handle::P1>(const Tri& tri){
  return tri.P1();
}

template<>
inline Point get<Handle::P2>(const Tri& tri){
  return tri.P2();
}

template<>
inline Point get<Handle::P3>(const Tri& tri){
  return tri.P3();
}

template<>
inline LineSegment get<Handle::P0P1>(const Tri& tri){
  return {tri.P0(), tri.P1()};
}

template<>
inline LineSegment get<Handle::P0P2>(const Tri& tri){
  return {tri.P0(), tri.P2()};
}

template<>
inline LineSegment get<Handle::P1P3>(const Tri& tri){
  return {tri.P1(), tri.P3()};
}

template<>
inline LineSegment get<Handle::P2P3>(const Tri& tri){
  return {tri.P2(), tri.P3()};
}

// Gets the Point from the Rect for the specified handle for corner
// handles, or a line between the corners for side handles.
template<Handle H>
typename handle_return<H>::type get(const Rect&){}

template<>
inline Point get<Handle::P0>(const Rect& rect){
  return rect.TopLeft();
}

template<>
inline Point get<Handle::P1>(const Rect& rect){
  return rect.TopRight();
}

template<>
inline Point get<Handle::P2>(const Rect& rect){
  return rect.BottomLeft();
}

template<>
inline Point get<Handle::P3>(const Rect& rect){
  return rect.BottomRight();
}

template<>
inline LineSegment get<Handle::P0P1>(const Rect& rect){
  return {rect.TopLeft(), rect.TopRight()};
}

template<>
inline LineSegment get<Handle::P0P2>(const Rect& rect){
  return {rect.TopLeft(), rect.BottomLeft()};
}

template<>
inline LineSegment get<Handle::P1P3>(const Rect& rect){
  return {rect.TopRight(), rect.BottomRight()};
}

template<>
inline LineSegment get<Handle::P2P3>(const Rect& rect){
  return {rect.BottomLeft(), rect.BottomRight()};
}

} // namespace

#endif
