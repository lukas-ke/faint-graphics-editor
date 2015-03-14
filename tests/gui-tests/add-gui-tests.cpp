#include "gui/dialog-context.hh"
#include "util/status-interface.hh"
#include "util-wx/fwd-wx.hh"

#define GUI_TEST_FUNCTION(NAME)void NAME(wxWindow*, faint::StatusInterface&, faint::DialogContext&)

GUI_TEST_FUNCTION(gui_test_bitmap_list_ctrl);
GUI_TEST_FUNCTION(gui_test_palette_ctrl);
GUI_TEST_FUNCTION(gui_test_selected_color_ctrl);
GUI_TEST_FUNCTION(gui_test_slider);
GUI_TEST_FUNCTION(gui_test_static_bitmap);
GUI_TEST_FUNCTION(gui_test_with_label);
GUI_TEST_FUNCTION(gui_test_resize_dialog);
GUI_TEST_FUNCTION(gui_test_alpha_dialog);

using test_init_func_t = void(*)(wxWindow*,
  faint::StatusInterface&,
  faint::DialogContext&);

static void add_test(const faint::utf8_string& name,
  test_init_func_t f,
  wxBookCtrlBase* pageList,
  faint::StatusInterface& statusInterface,
  faint::DialogContext& dialogContext)
{
  auto* p = faint::raw(faint::create_panel(pageList));
  f(p, statusInterface, dialogContext);
  add_page(pageList, p, name);
}

void add_gui_tests(wxBookCtrlBase* pageList,
  faint::StatusInterface& statusInterface,
  faint::DialogContext& dialogContext)
{
  add_test("AlphaDialog", gui_test_alpha_dialog,
    pageList, statusInterface, dialogContext);

  add_test("BitmapListCtrl", gui_test_bitmap_list_ctrl,
    pageList, statusInterface, dialogContext);

  add_test("PaletteCtrl", gui_test_palette_ctrl,
    pageList, statusInterface, dialogContext);

  add_test("ResizeDialog", gui_test_resize_dialog,
    pageList, statusInterface, dialogContext);

  add_test("SelectedColorCtrl", gui_test_selected_color_ctrl,
    pageList, statusInterface, dialogContext);

  add_test("Slider", gui_test_slider,
    pageList, statusInterface, dialogContext);

  add_test("StaticBitmap", gui_test_static_bitmap,
    pageList, statusInterface, dialogContext);

  add_test("WithLabel", gui_test_with_label,
    pageList, statusInterface, dialogContext);
}
