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

#ifndef FAINT_GUI_UTIL_HH
#define FAINT_GUI_UTIL_HH
#include <string>
#include <vector>
#include "wx/accel.h"
#include "wx/defs.h"
#include "app/resource-id.hh"
#include "formats/format.hh"
#include "geo/primitive.hh"
#include "gui/accelerator-entry.hh"
#include "gui/command-dialog.hh"
#include "gui/gui-string-types.hh"
#include "util/key-press.hh"

class wxBitmap;
class wxButton;
class wxCheckBox;
class wxColour;
class wxCursor;
class wxDialog;
class wxFileName;
class wxFrame;
class wxIcon;
class wxIconBundle;
class wxSize;
class wxStaticText;
class wxTextCtrl;
class wxWindow;

namespace faint{
class ArtContainer;

void console_message(const utf8_string&);

// Label or Bitmap button which inhibits the noise on keypress if
// button has focus
wxButton* noiseless_button(wxWindow* parent,
  const wxBitmap&,
  const Tooltip&,
  const wxSize&);

wxButton* noiseless_button(wxWindow* parent,
  const utf8_string&,
  const Tooltip&,
  const wxSize&);

// Retrieve the stored position and size etc. for the frame with the
// specified name
void restore_persisted_state(wxFrame*, const storage_name&);

wxColour get_gui_selected_color();

wxColour get_gui_deselected_color();

wxIcon get_icon(ArtContainer&, Icon);

wxIconBundle bundle_icons(const wxIcon&, const wxIcon&);

void fit_size_to(wxTextCtrl*, const wxString&);

int parse_int_value(wxTextCtrl*, int defaultValue);

coord parse_coord_value(wxTextCtrl*, coord defaultValue);

enum class SaveChoice{
  YES = wxID_YES,
  NO = wxID_NO,
  CANCEL = wxID_CANCEL
};

SaveChoice ask_close_unsaved_tab(wxWindow* parent, const Optional<FilePath>&);

bool ask_exit_unsaved_changes(wxWindow* parent);

void show_copy_color_error(wxWindow* parent);

void show_error(wxWindow* parent, const Title&, const utf8_string& message);
void show_error(wxWindow& parent, const Title&, const utf8_string& message);

// For showing an error message before Faint is fully initialized.
// Avoids trying to do modal-dialog handling and such.
// Always returns 'false' for convenience.
bool show_init_error(const Title&, const utf8_string&);

FileList show_open_file_dialog(wxWindow& parent,
  const Title&,
  const Optional<DirPath>&,
  const utf8_string& filter);

void show_file_not_found_error(wxWindow* parent, const FilePath&);

void show_file_not_supported_error(wxWindow* parent, const FilePath&);

void show_load_failed_error(wxWindow* parent,
  const FilePath&,
  const utf8_string& message);

void show_load_warnings(wxWindow* parents, const ImageProps&);

void show_out_of_memory_cancelled_error(wxWindow* parent);

// Toggles visibility of the frame: shows and raises it if hidden,
// restores if iconized, hides if shown.
template<typename T>
void toggle_top_level_window(T& window){
  if (window.IsHidden()){
    window.Show();
    window.Raise();
  }
  else if (window.IsIconized()){
    window.Restore();
    window.Raise();
  }
  else {
    window.Hide();
  }
}

inline wxWindow* null_parent(){
  return nullptr;
}

wxString get_new_canvas_title();

class AcceleratorEntry;
void set_accelerators(wxWindow*, const std::vector<AcceleratorEntry>&);

wxWindow* raw(wxCheckBox*);

wxWindow* raw(wxStaticText*);

} // namespace

#endif
