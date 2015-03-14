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

#ifndef FAINT_FWD_WX_HH
#define FAINT_FWD_WX_HH
#include <functional>
#include <vector>
#include "gui/gui-string-types.hh"
#include "util/distinct.hh"
#include "util-wx/window-types-wx.hh"

class wxCommandEvent;

namespace faint{

class Bitmap;
class Color;
class IntPoint;
class IntSize;
class utf8_string;

class category_fwd_wx;

using Width = Distinct<int, category_fwd_wx, 0>;
using FontSize = Distinct<int, category_fwd_wx, 1>;
using Bold = Distinct<bool, category_fwd_wx, 2>;

void destroy(window_t);
void destroy(wxDialog*);
wxDialog* center_over_parent(wxDialog*);
void center_over_parent(unique_dialog_ptr&);
void show(wxDialog*);
void show(unique_dialog_ptr&);
void set_bitmap(wxButton&, const wxBitmap&);
void set_bitmap(wxButton&, const Bitmap&);
IntSize get_size(const wxBitmap&);
int get_id(window_t);
void set_focus(window_t);

using button_fn = std::function<void()>;
using checkbox_fn = std::function<void()>;

wxButton* create_button(window_t parent,
  const char* label,
  const IntPoint& pos,
  const button_fn&);

wxButton* create_button(window_t parent,
  const char* label,
  const IntSize& initialSize,
  const button_fn&);

wxButton* create_button(window_t parent,
  const char* label,
  const Width&,
  const button_fn&);

wxButton* create_button(window_t,
  const wxBitmap&,
  const button_fn&);

wxButton* create_button(window_t,
  const wxBitmap&,
  const Tooltip&,
  const button_fn&);

wxButton* create_button(window_t,
  const wxBitmap&,
  const IntSize&,
  const button_fn&);

wxButton* create_button(window_t,
  const wxBitmap&,
  const IntSize&,
  const Tooltip&,
  const button_fn&);

wxButton* create_button(window_t,
  const char* label,
  const button_fn&);

wxButton* create_ok_button(window_t parent);
wxButton* create_cancel_button(window_t parent);
wxButton* create_cancel_button(window_t parent, const IntSize&);

wxStaticText* create_label(window_t parent,
  const utf8_string&,
  FontSize,
  Bold,
  const Color&);

wxStaticText* create_label(window_t parent,
  const char*,
  Bold,
  const Color&);

wxStaticText* create_label(window_t parent, const char*, const IntPoint&);

wxStaticText* create_label(window_t parent, const utf8_string&);

unique_dialog_ptr create_dialog(window_t parent,
  const utf8_string& title);

unique_dialog_ptr null_dialog();

wxTextCtrl* create_text_control(window_t parent, const char* initialText);

wxTextCtrl* create_text_control(window_t parent,
  const char*,
  const IntPoint&,
  const Width&);

wxTextCtrl* create_text_control(window_t parent, const Width&);
wxTextCtrl* create_text_control(window_t parent, const IntPoint&, const Width&);

wxTextCtrl* create_multiline_text_control(window_t parent, const IntSize&);

wxChoice* create_choice(window_t parent, const std::vector<utf8_string>&);

// Returns a sizer with OK and Cancel-buttons organized as
// expected on the current platform.
wxSizer* create_ok_cancel_buttons(wxDialog*);
wxSizer* create_ok_cancel_buttons(window_t parent, wxDialog* ancestor );

bool select(wxChoice*, const utf8_string&);

wxWindow* create_panel(window_t parent);

utf8_string get_text(wxTextCtrl*);
utf8_string get_text(wxChoice*);

void make_uniformly_sized(const std::initializer_list<window_t>&);

void set_sizer(window_t, wxSizer*);

void select_all(wxTextCtrl*);

void set_pos(window_t, const IntPoint&);
void set_size(window_t, const IntSize&);
IntPoint get_pos(window_t);
IntSize get_size(window_t);
int get_width(window_t);
int get_height(window_t);

// Converter to wxWindow* for any wxWidgets pointer that is implicitly
// convertible to window_t. This allows upcasting wxWidgets classes
// known only by a forward declaration so that they can be passed to
// functions expecting a wxWindow*.
wxWindow* raw(window_t);
wxWindow* raw(unique_dialog_ptr&);

wxWindow* create_hline(window_t parent);

wxCheckBox* create_checkbox(window_t parent, const char* label, bool checked);

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  const IntPoint&);

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  const IntPoint&);

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  checkbox_fn&&);

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  const IntPoint&,
  checkbox_fn&&);

bool get(wxCheckBox*);
bool not_set(wxCheckBox*);

// Unchecks the checkbox and sends a change event.
void clear(wxCheckBox*);

// Toggles the checked-state and sends a change event.
bool toggle(wxCheckBox*);

// Sets the checked-state and sends a change event if it changed.
void set(wxCheckBox*, bool);

class AcceleratorEntry;
void set_accelerators(wxWindow*, const std::vector<AcceleratorEntry>&);

void end_modal_ok(wxDialog*);

void set_tooltip(window_t, const Tooltip&);
void set_tooltip(window_t, const utf8_string&);

wxBookCtrlBase* create_notebook(window_t parent);
void add_page(wxBookCtrlBase*, window_t, const utf8_string&);
void set_selection(wxBookCtrlBase*, int);

wxWindow* create_hyperlink(window_t parent, const utf8_string url);

class wxWidgetsVersion{
public:
  wxWidgetsVersion();
  int majorVersion;
  int minorVersion;
  int release_number;
  int subrelease_number;
};

enum class Signal{YES, NO};

void set_number_text(wxTextCtrl*, int value, Signal);

void append_text(wxTextCtrl*, const utf8_string&);
bool is_empty(wxTextCtrl*);

void refresh(window_t);

void set_bgstyle_paint(window_t);
void set_initial_size(window_t, const IntSize&);
void set_background_color(window_t, const Color&);
Color get_background_color(window_t);
void set_cursor(window_t, int);

void process_event(window_t, wxCommandEvent&);

} // namespace

#endif
