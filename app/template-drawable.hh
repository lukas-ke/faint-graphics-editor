// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_TEMPLATE_DRAWABLE_HH
#define FAINT_TEMPLATE_DRAWABLE_HH
#include "rendering/paint-canvas.hh"
#include "objects/object.hh" // Fixme: Needed only for Drawable, add impl
#include "tools/tool.hh"
#include "util/setting-id.hh"
#include "util/pos-info.hh"
#include "text/text-expression-context.hh"
#include "app/canvas.hh" // Fixme: Pass Image instead
#include "util/image.hh"

namespace faint{

bool draw_before_zoom(const Tool& tool, Layer layer){
  return tool.DrawBeforeZoom(layer);
}

bool draw_before_zoom(const ExtraOverlay&, Layer){
  return true;
}

bool draw_before_zoom(const std::vector<Object*>&, Layer){
  return false;
}

bool should_draw_raster(const ExtraOverlay&, Layer){
  return false; // Fixme: Verify
}

bool should_draw_raster(const Tool& t, Layer l){
  return get_tool_layer(t.GetId(), l) == Layer::RASTER;
}

bool should_draw_raster(const std::vector<Object*>&, Layer){
  return false;
}

template<typename T>
void draw(T& obj, FaintDC& dc, Overlays& overlays, const PosInfo& info){
  obj.Draw(dc, overlays, info);
}

void draw(const std::vector<Object*>& objects,
  FaintDC& dc,
  Overlays&,
  const PosInfo& info)
{
  auto& expressionContext(info.canvas.GetImage().GetExpressionContext()); // Move to impl

  for (auto o : objects){
    o->Draw(dc, expressionContext);
  }
}

template<typename T>
class TemplateDrawable : public Drawable{
public:
  TemplateDrawable(T& obj)
    : m_obj(obj)
  {}

  void Draw(FaintDC& dc, Overlays& overlays, const PosInfo& info) override{
    draw(m_obj, dc, overlays, info);
  }

  bool DrawBeforeZoom(Layer layer) const override{
    return draw_before_zoom(m_obj, layer);
  }

  bool ShouldDrawRaster(Layer l) const override{
    return should_draw_raster(m_obj, l);
  }

  T& m_obj;
};

template<typename T>
auto template_drawable(T& obj){
  return TemplateDrawable<T>(obj);
}

} // namespace

#endif
