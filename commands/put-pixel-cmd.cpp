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

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "commands/command.hh"
#include "commands/put-pixel-cmd.hh"
#include "geo/int-point.hh"
#include "text/utf8-string.hh"

namespace faint{

class PutPixelCommand : public Command {
public:
  PutPixelCommand(const IntPoint& pos, const Color& color)
    : Command(CommandType::RASTER),
      m_color(color),
      m_pos(pos)
  {}

  void Do(CommandContext& context) override{
    put_pixel(context.GetRawBitmap(), m_pos, m_color);

  }
  utf8_string Name() const override{
    return "Set Pixel";
  }
private:
  Color m_color;
  IntPoint m_pos;
};

Command* put_pixel_command(const IntPoint& pos, const Color& color){
  return new PutPixelCommand(pos, color);
}

} // namespace
