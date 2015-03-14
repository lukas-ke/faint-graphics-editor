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

#include "bitmap/draw.hh"
#include "bitmap/paint.hh"
#include "commands/command.hh"
#include "commands/delete-rect-cmd.hh"
#include "geo/int-rect.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

class DeleteRectCommand : public Command{
public:
  DeleteRectCommand(const IntRect& rect,
    const Paint& bg,
    const Optional<Paint>& altBg,
    bool dwim)
    : Command(CommandType::RASTER),
      m_altBg(altBg),
      m_bg(bg),
      m_dwim(dwim),
      m_rect(rect)
  {}

  void Do(CommandContext& ctx) override{
    fill_rect(ctx.GetRawBitmap(), m_rect, m_bg);
  }

  Command* GetDWIM() override{
    return new DeleteRectCommand(m_rect, m_altBg.Get(), option(m_bg), true);
  }

  bool HasDWIM() const override{
    return m_altBg.IsSet();
  }

  utf8_string Name() const override{
    return m_dwim ?
      "Delete rectangle (DWIM)" :
      "Delete rectangle";
  }

  bool Same(const IntRect& rect, const Paint& bg) const{
    return m_rect == rect && m_bg == bg;
  }
private:
  Optional<Paint> m_altBg;
  Paint m_bg;
  bool m_dwim;
  IntRect m_rect;
};

Command* delete_rect_command(const IntRect& rect, const Paint& bg){
  return new DeleteRectCommand(rect, bg, no_option(), false);
}

Command* delete_rect_command(const IntRect& rect, const Paint& bg,
  const Alternative<Paint>& altBg)
{
  return new DeleteRectCommand(rect, bg, option(altBg.Get()), false);
}

} // namespace
