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
#include "commands/command.hh"
#include "commands/rescale-cmd.hh"
#include "geo/geo-func.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "util/command-util.hh"

namespace faint{

class RescaleCommand : public Command {
public:
  RescaleCommand(const IntSize& size, ScaleQuality quality)
    : Command(CommandType::HYBRID),
      m_size(size),
      m_oldSize(0,0), // Proper value set later
      m_quality(quality),
      m_objectResize(nullptr)
  {}

  ~RescaleCommand(){
    delete m_objectResize;
  }

  void Do(CommandContext& context) override{
    Size oldSize(floated(context.GetImageSize()));
    DoRaster(context);

    // Create the objectscale command on the first Do().
    if (m_objectResize == nullptr && context.HasObjects()){
      // Use the top left corner of the image as origin for object scaling.
      const Point origin(0,0);
      m_objectResize = get_scale_command(context.GetObjects(),
        Scale(New(floated(m_size)), oldSize), origin);
    }

    if (m_objectResize != nullptr){
      m_objectResize->Do(context);
    }
  }

  void DoRaster(CommandContext& context) override{
    const Bitmap& bmp(context.GetBitmap());
    m_oldSize = bmp.GetSize();
    Scale scaling(New(floated(m_size)), floated(m_oldSize));
    context.SetBitmap(scale(bmp, scaling, m_quality));
  }

  utf8_string Name() const override{
    return "Rescale Image";
  }

  Point Translate(const Point& p) const override{
    Size ratio(floated(m_size) / floated(m_oldSize));
    return p * point_from_size(ratio);
  }

  void Undo(CommandContext& context) override{
    if (m_objectResize != nullptr){
      m_objectResize->Undo(context);
    }
  }

  Point UndoTranslate(const Point& p) const override{
    Size invRatio(floated(m_oldSize) / floated(m_size));
    return p * point_from_size(invRatio);
  }

private:
  IntSize m_size;
  IntSize m_oldSize;
  ScaleQuality m_quality;
  Command* m_objectResize;
};

Command* rescale_command(const IntSize& size, ScaleQuality quality){
  return new RescaleCommand(size, quality);
}

} // namespace
