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

#include "commands/command.hh"
#include "commands/set-bitmap-cmd.hh"
#include "geo/int-point.hh"
#include "geo/tri.hh"
#include "objects/objraster.hh"
#include "text/utf8-string.hh"
#include "util/object-util.hh"

namespace faint{

class SetObjectBitmapCommand : public Command {
public:
  SetObjectBitmapCommand(ObjRaster* object,
    const Bitmap& bmp,
    const Tri& tri,
    const utf8_string& name)
    : Command(CommandType::OBJECT),
      m_object(object),
      m_bitmap(bmp),
      m_oldBitmap(object->GetBitmap()),
      m_tri(tri),
      m_oldTri(object->GetTri()),
      m_name(name)
  {}

  void Do(CommandContext&) override{
    m_object->SetBitmap(m_bitmap);
    m_object->SetTri(m_tri);
  }

  utf8_string Name() const override{
    return m_name;
  }

  void Undo(CommandContext&) override{
    m_object->SetBitmap(m_oldBitmap);
    m_object->SetTri(m_oldTri);
  }

  SetObjectBitmapCommand& operator=(const SetObjectBitmapCommand&) = delete;
private:
  ObjRaster* m_object;
  Bitmap m_bitmap;
  Bitmap m_oldBitmap;
  const Tri m_tri;
  const Tri m_oldTri;
  utf8_string m_name;
};

class SetBitmapCommand : public Command {
public:
  SetBitmapCommand(const Bitmap& bmp,
    const IntPoint& topLeft,
    const utf8_string& name)
    : Command(CommandType::HYBRID),
      m_bitmap(bmp),
      m_name(name),
      m_topLeft(topLeft)
  {}

  void Do(CommandContext& context) override{
    DoRaster(context);
    offset_by(context.GetObjects(), -m_topLeft);
  }

  void DoRaster(CommandContext& context) override{
    context.SetBitmap(m_bitmap);
  }

  utf8_string Name() const override{
    return m_name;
  }

  void Undo(CommandContext& context) override{
    offset_by(context.GetObjects(), m_topLeft);
  }

private:
  Bitmap m_bitmap;
  utf8_string m_name;
  IntPoint m_topLeft;
};

Command* set_object_bitmap_command(ObjRaster* obj,
  const Bitmap& bmp,
  const Tri& tri,
  const utf8_string& name)
{
  return new SetObjectBitmapCommand(obj, bmp, tri, name);
}

Command* set_bitmap_command(const Bitmap& bmp,
  const IntPoint& topLeft,
  const utf8_string& name)
{
  return new SetBitmapCommand(bmp, topLeft, name);
}

} // namespace
