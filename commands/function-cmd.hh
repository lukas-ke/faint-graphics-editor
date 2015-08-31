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

namespace faint{

class Bitmap;
class BitmapCommand;
class utf8_string;

BitmapCommand* function_command(const utf8_string& name,
  const std::function<void(Bitmap&)>&);

BitmapCommand* function_command(const utf8_string& name,
  std::function<void(Bitmap&)>&&);

template<typename Function, typename Arg, typename... Rest>
BitmapCommand* function_command(const utf8_string& name,
  Function&& f,
  Arg&& a,
  Rest&&... rest)
{
  // Bind all arguments of the function, except the bitmap parameter (the first
  // argument) which is provided as the only parameter when the
  // function is run as a command.
  return function_command(name,
    [=](Bitmap& bmp){
      return f(bmp, a, rest...);
    });
}

} // namespace

#endif
