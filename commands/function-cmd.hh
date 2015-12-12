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

#ifndef FAINT_FUNCTION_CMD_HH
#define FAINT_FUNCTION_CMD_HH
#include <functional>
#include "commands/bitmap-cmd.hh"

namespace faint{

class Bitmap;
class utf8_string;

BitmapCommandPtr function_command(const utf8_string& name,
  const std::function<void(Bitmap&)>&);

BitmapCommandPtr function_command(const utf8_string& name,
  std::function<void(Bitmap&)>&&);

template<typename... Args, typename... Rest>
BitmapCommandPtr in_place_function_command(const utf8_string& name,
  Bitmap(*func)(const Bitmap&, Args...),
  Rest&&... rest)
{
  return function_command(name,
    [=](Bitmap& bmp){
      bmp = func(bmp, rest...);
    });
}

template<typename... Rest, typename...Args>
BitmapCommandPtr function_command(const utf8_string& name,
  void(*func)(Bitmap&, Args...),
  Rest&&... rest)
{
  // Bind all arguments of the function, except the bitmap parameter (the first
  // argument) which is provided as the only parameter when the
  // function is run as a command.
  return function_command(name,
    [=](Bitmap& bmp){
      return func(bmp, rest...);
    });
}

template<typename ArgHere, typename ArgThere>
BitmapCommandPtr function_command(const utf8_string& name,
  void(*func)(Bitmap&, ArgThere),
  ArgHere&& a)
{
  // This function_command variant helps resolving the correct
  // bitmap-taking function in the face of overloads by requiring
  // matching arguments, instead of the compile failing on ambiguity.
  //
  // For example, with these overloads:
  //
  // 1. ColRGB blend_alpha(const Color&, const ColRGB&)
  // 2. void blend_alpha(Bitmap&, const ColRGB&)
  // 3. void blend_alpha(Bitmap&, const SpaceShip&)
  //
  // function_command(blend_alpha, ColRGB(255, 0, 0)) will correctly
  // resolve to #2.

  return function_command(name,
    [=](Bitmap& bmp){
      return func(bmp, a);
    });
}

template<typename Arg>
BitmapCommandPtr function_command(const utf8_string& name,
  void(*func)(Bitmap&, Arg),
  Arg&& a)
{
  // This function_command variant helps resolving the correct
  // bitmap-taking function in the face of overloads by requiring
  // matching arguments, instead of the compile failing on ambiguity.
  //
  // For example, with these overloads:
  //
  // 1. ColRGB blend_alpha(const Color&, const ColRGB&)
  // 2. void blend_alpha(Bitmap&, const ColRGB&)
  // 3. void blend_alpha(Bitmap&, const SpaceShip&)
  //
  // function_command(blend_alpha, ColRGB(255, 0, 0)) will correctly
  // resolve to #2.

  return function_command(name,
    [=](Bitmap& bmp){
      return func(bmp, a);
    });
}

} // namespace

#endif
