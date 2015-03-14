// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include <sstream>
#include "app/build-info.hh"
#include "gui/accelerator-entry.hh"
#include "gui/dialog-context.hh"
#include "gui/layout.hh"
#include "python/py-interface.hh" // For get_python_version
#include "rendering/cairo-context.hh" // For get_cairo_version
#include "text/formatting.hh"
#include "util-wx/file-path-util.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"

namespace faint{

constexpr Color titleColor(47, 54, 153);

static wxStaticText* title_text(wxWindow* parent){
  return create_label(parent,
    "Faint Graphics Editor",
    FontSize(24),
    Bold(true),
    titleColor);
}

static wxStaticText* version_text(wxWindow* parent){
   return create_label(parent,
     "Version " + faint_version(),
     FontSize(14),
     Bold(true),
     titleColor);
}

static wxStaticText* license_text(wxWindow* parent){
  return create_label(parent,
    "Copyright 2013 Lukas Kemmer\nLicensed under the Apache License 2.0");
}

static wxStaticText* details_text(wxWindow* parent){
  utf8_string faint_info =
    endline_sep(
      space_sep("Executable path:", get_faint_exe_path().Str()),
      space_sep("SVN path:",
        faint_svn_path() + ":" + faint_svn_revision()),
      space_sep("Build date:", faint_build_date()));

  std::stringstream ss;
  {
    auto v = wxWidgetsVersion();
    ss << "wxWidgets version: " <<
     v.majorVersion << "." <<
     v.minorVersion << "." <<
     v.release_number << "." <<
     v.subrelease_number << std::endl;
  }
  ss << "Python version: " << get_python_version() << std::endl;
  ss << "Cairo version: " << get_cairo_version() << std::endl;
  ss << "Pango version: " << get_pango_version() << std::endl;
  ss << std::endl;

  return create_label(parent, faint_info + "\n\n" + ss.str().c_str());
}

static auto create_about_dialog(wxWindow* parent){
  using namespace layout;

  auto dlg = create_dialog(parent, "About Faint");
  auto tabs = create_notebook(dlg.get());

  auto create_about_panel = [](wxWindow* tabs){
    auto panel = create_panel(raw(tabs));
    set_sizer(panel, create_row({{
      create_column({
        create_column(OuterSpacing(0), ItemSpacing(0), {
          raw(title_text(panel)),
          raw(version_text(panel))}),
        {create_hline(panel), Proportion(0), EXPAND|DOWN, item_spacing},
        {create_hyperlink(panel,
          "http://code.google.com/p/faint-graphics-editor/"),
          Proportion(0), DOWN, item_spacing},
        raw(license_text(panel))}),
      Proportion(1), EXPAND|DOWN, panel_padding}}));
    return panel;
  };

  auto create_details_panel = [](wxWindow* tabs){
    auto panel = create_panel(tabs);
    set_sizer(panel, create_row({{
      create_column({
        raw(details_text(panel))}),
      Proportion(1), EXPAND|UP|DOWN, panel_padding}}));

    return panel;
  };

  add_page(tabs, create_about_panel(raw(tabs)), "&About");
  add_page(tabs, create_details_panel(raw(tabs)), "&Details");

  set_sizer(dlg.get(), create_column(OuterSpacing(0), ItemSpacing(0), {
    raw(tabs),
    {make_default(dlg.get(), create_ok_button(dlg.get())), ALIGN_RIGHT}}));

  auto select_about_tab = [=](){set_selection(tabs, 0);};
  auto select_details_tab = [=](){set_selection(tabs, 1);};
  auto close_dialog = [dlg=dlg.get()](){end_modal_ok(dlg);};
  set_accelerators(raw(dlg.get()), {
    {key::A, select_about_tab},
    {Alt+key::A, select_about_tab},
    {key::Q, select_about_tab},

    {key::D, select_details_tab},
    {Alt+key::D, select_details_tab},
    {key::W, select_details_tab},

    {Alt+key::O, close_dialog}
  });
  return dlg;
}

void show_faint_about_dialog(wxWindow* parent, DialogContext& dialogContext){
  dialogContext.ShowModal(*create_about_dialog(parent));
}

} // namespace
