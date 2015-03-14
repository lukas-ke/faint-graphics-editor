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

#include "app/get-art-container.hh" // Fixme: Pass it instead
#include "app/app-context.hh"
#include "bitmap/bitmap.hh"
#include "app/canvas.hh"
#include "app/context-commands.hh"
#include "app/faint-resize-dialog-context.hh"
#include "commands/rescale-cmd.hh"
#include "commands/resize-cmd.hh"
#include "geo/geo-func.hh"
#include "gui/resize-dialog.hh"
#include "text/formatting.hh"
#include "util/apply-target.hh"
#include "util/command-util.hh"
#include "util/either.hh"
#include "util/object-util.hh"

namespace faint{

class FaintResizeDialogContext : public ResizeDialogContext{
public:
  FaintResizeDialogContext(AppContext& app)
    : m_app(app)
  {}

  bool AllowNearestNeighbour() const override{
    return dispatch_target(get_apply_target(m_app.GetActiveCanvas()),
      [](const OBJECT_SELECTION){return false;},
      [](const RASTER_SELECTION){return true;},
      [](const IMAGE){return true;});
  }

  bool AllowResize() const override{
    return dispatch_target(get_apply_target(m_app.GetActiveCanvas()),
      [](const OBJECT_SELECTION){return false;},
      [](const RASTER_SELECTION){return false;},
      [](const IMAGE){return true;});
  }

  Paint GetBackground() const override{
    return m_app.GetToolSettings().Get(ts_Bg);
  }

  ResizeDialogOptions GetOptions() const override{
    return m_app.GetDefaultResizeDialogOptions();
  }

  Either<IntSize, Size> GetSize() const override{
    const auto& canvas(m_app.GetActiveCanvas());
    ApplyTarget t(get_apply_target(canvas));
    return get_apply_target_size(canvas, t);
  }

  utf8_string GetTitle() const override{
    const auto& canvas(m_app.GetActiveCanvas());
    ApplyTarget t(get_apply_target(canvas));

    return dispatch_target<utf8_string>(t,
      [&](OBJECT_SELECTION){
        return space_sep("Resize ",
          get_collective_type(canvas.GetObjectSelection()));
      },
      [&](RASTER_SELECTION){ return "Scale Selection"; },
      [&](IMAGE){ return "Resize or Scale Image"; });
  }

  void Resize(const IntRect& r, const Paint& bg) override{
    auto& canvas(m_app.GetActiveCanvas());
    canvas.RunCommand(resize_command(r, bg));
  }

  void Rescale(Size size, ScaleQuality quality) override{
    auto& canvas(m_app.GetActiveCanvas());
    dispatch_target(get_apply_target(canvas),
      [&](OBJECT_SELECTION){
        canvas.RunCommand(context_scale_objects(canvas, size));
      },
      [&](RASTER_SELECTION){
        canvas.RunCommand(get_scale_raster_selection_command(canvas.GetImage(),
          rounded(size), // Fixme
          quality));
      },
      [&](IMAGE){
        canvas.RunCommand(rescale_command(rounded(size), quality));
      });
  }

  void SetDefaultOptions(const ResizeDialogOptions& options) override{
    m_app.SetDefaultResizeDialogOptions(options);
  }

  bool SupportedSize(const Size& size) override{
    return reasonable_bitmap_size(size);
  }

  FaintResizeDialogContext& operator=(const FaintResizeDialogContext&) = delete;
private:
  AppContext& m_app;
};

void show_resize_dialog(AppContext& app){
  show_resize_dialog(app.GetDialogContext(),
    get_art_container(),
    std::make_unique<FaintResizeDialogContext>(app));
}

} // namespace
