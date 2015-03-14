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

#ifndef FAINT_OBJRASTER_HH
#define FAINT_OBJRASTER_HH
#include "bitmap/bitmap.hh"
#include "geo/tri.hh"
#include "objects/object.hh"

namespace faint{

class ObjRaster : public Object {
public:
  ObjRaster(const Tri&, const Bitmap&, const Settings&);
  Object* Clone() const override;
  void Draw(FaintDC&, ExpressionContext&) override;
  void Draw(FaintDC&);
  void DrawMask(FaintDC&) override;
  std::vector<Point> GetAttachPoints() const override;
  Bitmap& GetBitmap();
  std::vector<PathPt> GetPath(const ExpressionContext&) const override;
  IntRect GetRefreshRect() const override;
  Tri GetTri() const override;
  utf8_string GetType() const override;
  bool HitTest(const Point&) override;
  void SetBitmap(const Bitmap&);
  void SetTri(const Tri&) override;
private:
  ObjRaster(const ObjRaster&); // For clone
  Bitmap m_bitmap;
  Bitmap m_scaled;
  Tri m_tri;
};

Tri tri_for_bmp(const Point& topLeft, const Bitmap&);

class Command;
Command* crop_raster_object_command(ObjRaster*);

} // namespace

#endif
