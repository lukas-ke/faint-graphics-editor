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
#include "commands/flip-rotate-cmd.hh"
#include "geo/axis.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/point.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "geo/size.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "text/formatting.hh"

namespace faint{

static void flip_horizontally(const objects_t& objects, const Size& size){
  const Point p0(size.w / 2.0f, size.h / 2.0f);
  for (Object* obj : objects){
    obj->SetTri(scaled(obj->GetTri(), invert_x_scale(), p0));
  }
}

static void flip_vertically(const objects_t& objects, const Size& size){
  const Point p0(size.w / 2.0f, size.h / 2.0f);
  for (Object* obj : objects){
    obj->SetTri(scaled(obj->GetTri(), invert_y_scale(), p0));
  }
}

class FlipImageCommand : public Command{
public:
  FlipImageCommand(Axis axis) :
    Command(CommandType::HYBRID),
    m_axis(axis)
  {}

  void Do(CommandContext& context) override{
    DoObjects(context);
    DoRaster(context);
  }

  void DoRaster(CommandContext& context) override{
    const Bitmap& bmp(context.GetBitmap());
    Point p0(static_cast<coord>(bmp.m_w) / 2.0, static_cast<coord>(bmp.m_h) / 2.0);
    context.SetBitmap(flip(bmp, along(m_axis)));
  }

  utf8_string Name() const override{
    return m_axis == Axis::HORIZONTAL ?
      "Horizontal Image Flip" : "Vertical Image Flip";
  }

  void Undo(CommandContext& context) override{
    // Flipping again undoes object changes
    DoObjects(context);
  }
private:
  void DoObjects(CommandContext& context){
    if (m_axis == Axis::HORIZONTAL){
      flip_horizontally(context.GetObjects(), floated(context.GetImageSize()));
    }
    else if (m_axis == Axis::VERTICAL){
      flip_vertically(context.GetObjects(), floated(context.GetImageSize()));
    }
    else{
      assert(false);
    }
  }
  Axis m_axis;
};

class RotateImage90Command : public Command {
public:
  RotateImage90Command()
    : Command(CommandType::HYBRID)
  {}

  void Do(CommandContext& context) override{
    DoObjects(context);
    DoRaster(context);
  }

  void DoRaster(CommandContext& context) override{
    const Bitmap& bmp(context.GetBitmap());
    Point p0(static_cast<coord>(bmp.m_w) / 2.0, static_cast<coord>(bmp.m_h) / 2.0);
    context.SetBitmap(rotate_90cw(bmp));
  }

  utf8_string Name() const override{
    return space_sep("Rotate Image", str_degrees_int_symbol(90));
  }

  void Undo(CommandContext& context) override{
    // Undo object changes
    const Size size(floated(context.GetImageSize()));
    const objects_t& objects = context.GetObjects();
    Point p0(size.w, size.h);
    for (Object* obj : objects){
      obj->SetTri(translated(rotated(obj->GetTri(), -(pi / 2), p0),
          -size.w + size.h, - size.h));
    }
  }
private:
  void DoObjects(CommandContext& context){
    const Size size(floated(context.GetImageSize()));
    const objects_t& objects = context.GetObjects();
    Point p0(size.w, size.h);
    for (Object* obj : objects){
      obj->SetTri(translated(rotated(obj->GetTri(), pi / 2, p0),
          -size.w, size.w - size.h));
    }
  }
};

static Point get_object_offset(const Rect& imageRect, const Angle& angle){
  Tri tri = rotated(tri_from_rect(imageRect), angle, imageRect.Center());
  return bounding_rect(tri).TopLeft();
}

class RotateImageCommand : public Command {
public:
  RotateImageCommand(const Angle& angle, const Paint& bg)
    : Command(CommandType::HYBRID),
      m_angle(angle),
      m_bg(bg)
  {}

  RotateImageCommand(const Angle& angle, const Paint& bg, const AltPaint& altBg)
    : Command(CommandType::HYBRID),
      m_altBg(altBg.Get()),
      m_angle(angle),
      m_bg(bg)
  {}

  bool HasDWIM() const override{
    return m_altBg.IsSet();
  }

  void Do(CommandContext& ctx) override{
    Rect rect = floated(rect_from_size(ctx.GetImageSize()));
    m_offset = get_object_offset(rect, m_angle);

    for (Object* obj : ctx.GetObjects()){
      obj->SetTri(translated(rotated(obj->GetTri(), m_angle, rect.Center()),
        -m_offset.x, -m_offset.y));
    }
    DoRaster(ctx);
  }

  void DoRaster(CommandContext& ctx) override{
    ctx.SetBitmap(rotate_bilinear(ctx.GetBitmap(), m_angle, m_bg));
  }

  Command* GetDWIM() override{
    assert(m_altBg.IsSet());
    return new RotateImageCommand(m_angle, m_altBg.Get(), alternate(m_bg));
  }

  utf8_string Name() const override{
    return space_sep("Rotate Image", str_degrees_symbol(m_angle));
  }

  void Undo(CommandContext& ctx) override{
    Size oldSize = floated(ctx.GetImageSize());
    const objects_t& objects = ctx.GetObjects();
    Point center = point_from_size(oldSize) / 2;
    for (Object* obj : objects){
      obj->SetTri(translated(rotated(obj->GetTri(), -m_angle, center),
          m_offset.x, m_offset.y));
    }
  }
private:
  Optional<Paint> m_altBg;
  Angle m_angle;
  Paint m_bg;
  Point m_offset;
};

Command* rotate_image_command(const Angle& angle, const Paint& bg){
  return new RotateImageCommand(angle, bg);
}

Command* rotate_image_command(const Angle& angle, const Paint& bg,
  const AltPaint& altBg)
{
  return altBg.Get() == bg ?
    new RotateImageCommand(angle, bg) :
    new RotateImageCommand(angle, bg, altBg);
}

Command* rotate_image_90cw_command(){
  return new RotateImage90Command();
}

Command* flip_image_command(Axis axis){
  return new FlipImageCommand(axis);
}

static Point get_object_offset(const Rect& imageRect,
  const Angle& angle,
  const Scale& scale)
{
  const Point c = imageRect.Center();
  return bounding_rect(rotated(
    scaled(tri_from_rect(imageRect), scale, c),
      angle, c)).TopLeft();
}

class RotateScaleImageCommand : public Command {
public:
  RotateScaleImageCommand(const Angle& angle, const Scale& scale, const Paint& bg)
    : Command(CommandType::HYBRID),
      m_angle(angle),
      m_bg(bg),
      m_scale(scale)
  {}

  void Do(CommandContext& ctx) override{
    Rect rect = floated(rect_from_size(ctx.GetImageSize()));
    m_offset = get_object_offset(rect, m_angle, m_scale);

    for (Object* obj : ctx.GetObjects()){
      obj->SetTri(translated(rotated(scaled(obj->GetTri(), m_scale, rect.Center()), m_angle, rect.Center()), -m_offset.x, -m_offset.y));
    }
    DoRaster(ctx);
  }

  void DoRaster(CommandContext& ctx) override{
    ctx.SetBitmap(rotate_scale_bilinear(ctx.GetBitmap(), m_angle, m_scale, m_bg));
  }

  utf8_string Name() const override{
    return space_sep("Rotate",
      str_degrees_symbol(m_angle),
      "Scale", str(m_scale));
  }

  void Undo(CommandContext& ctx) override{
    Size oldSize = floated(ctx.GetImageSize());
    const objects_t& objects = ctx.GetObjects();
    Point center = point_from_size(oldSize) / 2;
    for (Object* obj : objects){
      obj->SetTri(translated(rotated(scaled(obj->GetTri(), inverse(m_scale), center), -m_angle, center), m_offset.x, m_offset.y));
    }
  }
private:
  Angle m_angle;
  Paint m_bg;
  Point m_offset;
  Scale m_scale;
};

Command* rotate_scale_image_bilinear_command(const Angle& angle,
  const Scale& scale,
  const Paint& bg)
{
  return new RotateScaleImageCommand(angle, scale, bg);
}

} // namespace
