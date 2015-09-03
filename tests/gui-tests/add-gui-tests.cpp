#include <functional>
#include "gui/dialog-context.hh"
#include "util-wx/fwd-wx.hh"
#include "util/status-interface.hh"

namespace faint{ class Art; }

#define GUI_TEST_FUNCTION(NAME)void NAME(wxWindow*, faint::StatusInterface&, faint::DialogContext&)

#define GUI_TEST_FUNCTION_ART(NAME)void NAME(wxWindow*, faint::StatusInterface&, faint::DialogContext&, const faint::Art&)

GUI_TEST_FUNCTION(gui_test_alpha_dialog);
GUI_TEST_FUNCTION(gui_test_gradient_slider);
GUI_TEST_FUNCTION_ART(gui_test_bitmap_list_ctrl);
GUI_TEST_FUNCTION_ART(gui_test_frame_ctrl);
GUI_TEST_FUNCTION(gui_test_gradient_panel);
GUI_TEST_FUNCTION(gui_test_grid_dialog);
GUI_TEST_FUNCTION(gui_test_hsl_panel);
GUI_TEST_FUNCTION_ART(gui_test_image_toggle_ctrl);
GUI_TEST_FUNCTION(gui_test_palette_ctrl);
GUI_TEST_FUNCTION(gui_test_pattern_panel);
GUI_TEST_FUNCTION_ART(gui_test_resize_dialog);
GUI_TEST_FUNCTION(gui_test_selected_color_ctrl);
GUI_TEST_FUNCTION(gui_test_slider);
GUI_TEST_FUNCTION(gui_test_static_bitmap);
GUI_TEST_FUNCTION(gui_test_with_label);

using test_init_func_t = std::function<void(wxWindow*,
  faint::StatusInterface&,
  faint::DialogContext&)>;

using test_init_func_art_t = std::function<void(wxWindow*,
  faint::StatusInterface&,
  faint::DialogContext&,
  const faint::Art&)>;

void add_gui_tests(wxBookCtrlBase* pageList,
  faint::StatusInterface& statusInterface,
  faint::DialogContext& dialogContext,
  const faint::Art& art)
{
  auto add_test = [&](const faint::utf8_string& name, test_init_func_t f){
    auto* p = faint::raw(faint::create_panel(pageList));
    f(p, statusInterface, dialogContext);
    add_page(pageList, p, name);
  };

  auto add_test_art = [&](const faint::utf8_string& name, test_init_func_art_t f){
    add_test(name,
      [&](wxWindow* w, faint::StatusInterface& status, faint::DialogContext& ctx){
        f(w, status, ctx, art);
      });
  };

  add_test("AlphaDialog", gui_test_alpha_dialog);
  add_test_art("BitmapListCtrl", gui_test_bitmap_list_ctrl);
  add_test_art("FrameCtrl", gui_test_frame_ctrl);
  add_test("GradientPanel", gui_test_gradient_panel);
  add_test("GradientSlider", gui_test_gradient_slider);
  add_test("GridDialog", gui_test_grid_dialog);
  add_test("HSLPanel", gui_test_hsl_panel);
  add_test_art("ImageToggleCtrl", gui_test_image_toggle_ctrl);
  add_test("PaletteCtrl", gui_test_palette_ctrl);
  add_test("PatternPanel", gui_test_pattern_panel);
  add_test_art("ResizeDialog", gui_test_resize_dialog);
  add_test("SelectedColorCtrl", gui_test_selected_color_ctrl);
  add_test("Slider", gui_test_slider);
  add_test("StaticBitmap", gui_test_static_bitmap);
  add_test("WithLabel", gui_test_with_label);
}
