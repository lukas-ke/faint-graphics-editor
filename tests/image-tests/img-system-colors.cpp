// -*- coding: us-ascii-unix -*-
#include "test-sys/test-name.hh"
#include "tests/test-util/image-table.hh"
#include "tests/test-util/file-handling.hh"
#include "bitmap/bitmap.hh"
#include "bitmap/draw.hh"
#include "util-wx/system-colors.hh"
#include "geo/line.hh"
#include "geo/int-rect.hh"
#include "text/formatting.hh"

#define COLOR_ROW(func)\
t.AddRow();\
 t.AddCell(save_test_image(faint::Bitmap(IntSize(32,32), func()), FileName(#func ".png"))); \
t.AddCell(utf8_string(#func)); \
t.AddCell(str_rgb(func()));

void img_system_colors(){
  using namespace faint;

  ImageTable t(get_test_name(), {"Color", "Function", "RGB"});
  COLOR_ROW(color_active_border);
  COLOR_ROW(color_active_caption);
  COLOR_ROW(color_app_workspace);
  COLOR_ROW(color_button_face);
  COLOR_ROW(color_button_highlight);
  COLOR_ROW(color_button_shadow);
  COLOR_ROW(color_button_text);
  COLOR_ROW(color_captiontext);
  COLOR_ROW(color_dark_shadow_3d);
  COLOR_ROW(color_desktop);
  COLOR_ROW(color_gradient_active_caption);
  COLOR_ROW(color_gradient_inactive_caption);
  COLOR_ROW(color_graytext);
  COLOR_ROW(color_highlight);
  COLOR_ROW(color_highlighttext);
  COLOR_ROW(color_hotlight);
  COLOR_ROW(color_inactive_border);
  COLOR_ROW(color_inactive_caption);
  COLOR_ROW(color_inactive_caption_text);
  COLOR_ROW(color_info_background);
  COLOR_ROW(color_info_text);
  COLOR_ROW(color_listbox);
  COLOR_ROW(color_listbox_highlight_text);
  COLOR_ROW(color_listbox_text);
  COLOR_ROW(color_menu);
  COLOR_ROW(color_menu_highlight);
  COLOR_ROW(color_menubar);
  COLOR_ROW(color_menutext);
  COLOR_ROW(color_system_face_3d);
  COLOR_ROW(color_system_light_3d);
  COLOR_ROW(color_system_scrollbar);
  COLOR_ROW(color_window);
  COLOR_ROW(color_windowframe);
  COLOR_ROW(color_windowtext);
  save_image_table(t);
}
