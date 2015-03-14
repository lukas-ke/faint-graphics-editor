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

#include "commands/bitmap-cmd.hh"
#include "commands/function-cmd.hh"
#include "text/utf8-string.hh"

namespace faint{

class FunctionCommand : public BitmapCommand {
 public:
  FunctionCommand(const utf8_string& name,
    const std::function<void(Bitmap&)>& func)
    : m_name(name),
      m_func(func)
  {}

  utf8_string Name() const override{
    return m_name;
  }
  void Do(Bitmap& bmp) override{
    m_func(bmp);
  }

private:
  utf8_string m_name;
  std::function<void(Bitmap&)> m_func;
};

BitmapCommand* get_function_command(const utf8_string& name,
  const std::function<void(Bitmap&)>& func)
{
  return new FunctionCommand(name, func);
}

} // namespace
