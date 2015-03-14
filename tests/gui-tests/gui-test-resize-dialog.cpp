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

#include "app/get-art-container.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "gui/resize-dialog.hh"
#include "text/formatting.hh"
#include "util/optional.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"

faint::ResizeDialogOptions g_resizeDialogOptions;

namespace faint{ class StatusInterface; }

namespace{

using namespace faint;

static utf8_string to_str(ResizeDialogOptions::ResizeType type){
  switch(type){
  case ResizeDialogOptions::RESCALE:
    return "RESCALE";
  case ResizeDialogOptions::RESIZE_TOP_LEFT:
    return "RESIZE_TOP_LEFT";
  case ResizeDialogOptions::RESIZE_CENTER:
    return "RESIZE_CENTER";
  default:
    assert(false);
    return "";
  }
}

class TestResizeDialogContext : public faint::ResizeDialogContext {
public:
  TestResizeDialogContext(const ResizeDialogOptions& options,
    wxTextCtrl* log,
    const utf8_string& title,
    const Paint& bg,
    bool allowNearest,
    bool allowResize)
    : m_allowNearest(allowNearest),
      m_allowResize(allowResize),
      m_bg(bg),
      m_log(log),
      m_options(options),
      m_title(title)
  {}

  bool AllowNearestNeighbour() const override{
    return m_allowNearest;
  }

  bool AllowResize() const override{
    return m_allowResize;
  }

  Paint GetBackground() const override{
    return m_bg;
  }

  ResizeDialogOptions GetOptions() const override{
    return m_options;
  }

  void SetDefaultOptions(const ResizeDialogOptions& o) override{
    m_options = o;
    auto s(no_sep("SetDefaultOptions", bracketed(
      comma_sep(to_str(o.defaultButton),
        lbl("nearest", o.nearest? "true" : "false"),
        lbl("proportional", o.proportional ? "true" : "false")))));

    append_text(m_log, s + "\n");
  }

  utf8_string GetTitle() const override{
    return m_title;
  }

  Either<IntSize, Size> GetSize() const override{
    return Size(640, 480);
  }

  void Resize(const IntRect& r, const Paint& bg) override{
    auto s(no_sep("Resize", bracketed(
      comma_sep(str(r.TopLeft()), str(r.GetSize()), str(bg)))));
    append_text(m_log, s + "\n");
  }

  void Rescale(Size sz, ScaleQuality quality) override{
    auto s(no_sep("Rescale", bracketed(
      comma_sep(str(sz, 2_dec),
        quality == ScaleQuality::BILINEAR ?
        "Bilinear" :
        "Nearest"))));
    append_text(m_log, s + "\n");
  }

  bool SupportedSize(const Size&) override{
    return true;
  }

  bool m_allowNearest;
  bool m_allowResize;
  Paint m_bg;
  wxTextCtrl* m_log;
  ResizeDialogOptions m_options;
  utf8_string m_title;
};

static auto make_ctx(wxTextCtrl* log,
  const utf8_string& title,
  const Paint& bg,
  bool allowNearest,
  bool scaleOnly){
  return std::make_unique<TestResizeDialogContext>(
    ResizeDialogOptions(),
    log,
    title,
    bg,
    allowNearest,
    scaleOnly);
}

} // namespace

void gui_test_resize_dialog(wxWindow* p, faint::StatusInterface&,
  faint::DialogContext& dialogContext)
{
  using namespace faint;

  auto log = faint::create_multiline_text_control(p, IntSize(400, 400));

  using namespace layout;
  auto dialog_button =
    [=, &dialogContext](const utf8_string& title,
      const Paint& bg,
      bool allowNearest,
      bool allowResize)
    {
      auto b = create_button(p, title.c_str(),
        [=, &dialogContext](){
          if (!is_empty(log)){
            append_text(log, "\n");
          }
          append_text(log, "Show: " + title + "\n");
          show_resize_dialog(dialogContext,
            get_art_container(),
            make_ctx(log, title, bg, allowNearest, allowResize));
        });
      return b;
    };

  set_sizer(p, create_column({
    dialog_button("Full image", Paint(Color(120, 120, 0, 120)), true, true),
    dialog_button("Object", Paint(Color(120, 120, 0, 120)), false, false),
    dialog_button("Raster selection", Paint(Color(120, 120, 0, 120)), true, false),
    raw(log)}));
}
