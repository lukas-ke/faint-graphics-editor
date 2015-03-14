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

#include <cassert>
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "commands/command.hh"
#include "commands/resize-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "text/utf8-string.hh"
#include "util/optional.hh"

namespace faint{

class ResizeCommand : public Command {
public:
  ResizeCommand(const IntRect& rect, const Paint& bg, const utf8_string& name)
    : Command(CommandType::HYBRID),
      m_bg(bg),
      m_name(name),
      m_rect(rect)
  {}

  ResizeCommand(const IntRect& rect, const AltIntRect& altRect,
    const Paint& bg, const utf8_string& name)
    : Command(CommandType::HYBRID),
      m_altRect(altRect.Get()),
      m_bg(bg),
      m_name(name),
      m_rect(rect)
  {}

  ResizeCommand(const IntRect& rect, const Paint& bg, const AltPaint& altBg,
    const utf8_string& name)
    : Command(CommandType::HYBRID),
      m_altBg(altBg.Get()),
      m_bg(bg),
      m_name(name),
      m_rect(rect)
  {}

  void Do(CommandContext& context) override{
    DoRaster(context);
    context.OffsetOrigin(-m_rect.TopLeft());
  }

  void DoRaster(CommandContext& context) override{
    Bitmap bmp(m_rect.GetSize(), m_bg);
    blit(offsat(context.GetBitmap(), -m_rect.TopLeft()), onto(bmp));
    context.SetBitmap(std::move(bmp));
  }

  Command* GetDWIM() override{
    if (m_altBg.IsSet()){
      return new ResizeCommand(m_rect, m_altBg.Get(),
        alternate(m_bg), "Resize Image (DWIM)");
    }
    else if (m_altRect.IsSet()){
      return new ResizeCommand(m_altRect.Get(),
        alternate(m_rect), m_bg, "Resize Image (DWIM)");
    }
    assert(false);
    return nullptr;
  }

  bool HasDWIM() const override{
    return m_altBg.IsSet() || m_altRect.IsSet();
  }

  utf8_string Name() const override{
    return m_name;
  }

  Point Translate(const Point& p) const override{
    return p - floated(m_rect.TopLeft());
  }

  void Undo(CommandContext& context) override{
    context.OffsetOrigin(m_rect.TopLeft());
  }

  Point UndoTranslate(const Point& p) const override{
    return p + floated(m_rect.TopLeft());
  }

private:
  Optional<Paint> m_altBg;
  Optional<IntRect> m_altRect;
  Paint m_bg;
  utf8_string m_name;
  IntRect m_rect;
};

Command* resize_command(const IntRect& rect, const Paint& bg,
  const utf8_string& name)
{
  return new ResizeCommand(rect, bg, name);
}

Command* resize_command(const IntRect& rect, const AltIntRect& altRect,
  const Paint& bg, const utf8_string& name)
{
  return new ResizeCommand(rect, altRect, bg, name);
}

Command* resize_command(const IntRect& rect, const Paint& bg,
  const AltPaint& altBg, const utf8_string& name)
{
  return new ResizeCommand(rect, bg, altBg, name);
}

} // namespace
