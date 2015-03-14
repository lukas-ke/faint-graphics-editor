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

#include "wx/wx.h"
#include "wx/bookctrl.h"
#include "wx/hyperlink.h"
#include "wx/statline.h"
#include "bitmap/color.hh"
#include "geo/int-point.hh"
#include "geo/int-size.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/bind-event.hh"

namespace faint{

static auto to_wx(const Width& w){
  return wxSize(w.Get(), -1);
}

window_t::window_t(wxDialog* dlg) : w(dlg){}
window_t::window_t(unique_dialog_ptr& dlg) : w(dlg.get()){}
window_t::window_t(wxWindow* wnd) : w(wnd){}
window_t::window_t(wxWindow& wnd) : w(&wnd){}
window_t::window_t(wxBookCtrlBase* wnd) : w(wnd){}
window_t::window_t(wxPanel* panel) : w(panel){}
window_t::window_t(wxCheckBox* cb) : w(cb){}
window_t::window_t(wxStaticText* txt) : w(txt){}
window_t::window_t(wxTextCtrl* txt) : w(txt){}
window_t::window_t(wxButton* button) : w(button){}

void destroy(window_t w){
  w.w->Destroy();
}
void set_bitmap(wxButton& button, const wxBitmap& bmp){
  button.SetBitmap(bmp);
}

void set_bitmap(wxButton& button, const Bitmap& bmp){
  button.SetBitmap(to_wx_bmp(bmp));
}

void show(wxDialog* dlg){
  dlg->Show();
}

void show(unique_dialog_ptr& dlg){
  dlg->Show();
}

int show_modal(wxDialog* dlg){
  return dlg->ShowModal();
}

IntSize get_size(const wxBitmap& bmp){
  return to_faint(bmp.GetSize());
}

int get_id(window_t w){
  return w.w->GetId();
}

IntSize get_size(window_t w){
  return to_faint(w.w->GetSize());
}

void set_initial_size(window_t w, const IntSize& size){
  w.w->SetInitialSize(to_wx(size));
}

wxButton* create_button(window_t parent,
  const char* label,
  const button_fn& f)
{
  return bind(new wxButton(parent.w, wxID_ANY, label), wxEVT_BUTTON, f);
}

wxButton* create_button(window_t parent,
  const char* label,
  const Width& width,
  const button_fn& f)
{
  return bind(new wxButton(parent.w, wxID_ANY,
      label, wxDefaultPosition, to_wx(width)),
    wxEVT_BUTTON, f);
}

wxCheckBox* create_checkbox(window_t parent, const char* label, bool checked){
  auto* c = new wxCheckBox(parent.w, wxID_ANY, label);
  c->SetValue(checked);
  return c;
}

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  const IntPoint& pos)
{
  return new wxCheckBox(parent.w, wxID_ANY, label, to_wx(pos));
}

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  const IntPoint& pos)
{
  auto c = new wxCheckBox(parent.w, wxID_ANY, label, to_wx(pos));
  c->SetValue(checked);
  return c;
}

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  const IntPoint& pos,
  checkbox_fn&& f)
{
  auto c = create_checkbox(parent, label, checked, pos);
  c->Bind(wxEVT_CHECKBOX,
    [f = std::move(f)](const wxCommandEvent&){
      f();
    });
  return c;
}

wxCheckBox* create_checkbox(window_t parent,
  const char* label,
  bool checked,
  checkbox_fn&& f)
{
  wxCheckBox* cb = create_checkbox(parent.w, label, checked);
  cb->Bind(wxEVT_CHECKBOX,
    [f = std::move(f)](const wxCommandEvent&){
      f();
    });
  return cb;
}

wxChoice* create_choice(window_t parent,
  const std::vector<utf8_string>& strs)
{
  std::vector<wxString> strsWx;
  for (const auto& s : strs){
    strsWx.push_back(to_wx(s));
  }

  return new wxChoice(parent.w,
    wxID_ANY,
    wxDefaultPosition, wxDefaultSize,
    static_cast<int>(strsWx.size()), strsWx.data()); // Fixme: Check cast
}

void delete_dialog(wxDialog*& dlg){
  if (dlg != nullptr){
    dlg->Destroy();
    dlg = nullptr;
  }
}

std::unique_ptr<wxDialog, delete_dialog_f> create_dialog(window_t parent,
  const utf8_string& title)
{
  return std::unique_ptr<wxDialog, delete_dialog_f>(
    center_over_parent(
      new wxDialog(parent.w, wxID_ANY, to_wx(title),
        wxDefaultPosition, wxDefaultSize,
        wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER)),
    destroy);
}

wxSizer* create_ok_cancel_buttons(window_t parent, wxDialog* dlg){
  auto okButton = create_ok_button(parent);
  auto cancelButton = create_cancel_button(parent);
  auto sizer = new wxStdDialogButtonSizer();
  sizer->Add(okButton);
  sizer->Add(cancelButton);
  sizer->Realize();
  dlg->SetDefaultItem(okButton);
  return sizer;
}

wxSizer* create_ok_cancel_buttons(wxDialog* dlg){
  return create_ok_cancel_buttons(dlg, dlg);
}

wxWindow* create_panel(window_t parent){
  return new wxPanel(parent.w, wxID_ANY);
}

wxButton* create_button(window_t parent,
  const char* label,
  const IntSize& initialSize,
  const button_fn& f)
{
  wxButton* b = create_button(parent, label, f);
  b->SetInitialSize(to_wx(initialSize));
  return b;
}

static wxSize largest_size(const std::initializer_list<window_t>& controls){
  int maxWidth = 0;
  int maxHeight = 0;
  for (auto& w : controls){
    wxSize sz(w.w->GetSize());
    maxWidth = std::max(sz.GetWidth(), maxWidth);
    maxHeight = std::max(sz.GetHeight(), maxHeight);
  }
  return wxSize(maxWidth + 10, maxHeight);
}

void make_uniformly_sized(const std::initializer_list<window_t>& controls){
  wxSize size(largest_size(controls));
  for (auto& w : controls){
    w.w->SetInitialSize(size);
  }
}

wxButton* create_button(window_t w,
  const wxBitmap& bmp,
  const IntSize& size,
  const button_fn& f)
{
  wxButton* button = new wxButton(w.w,
    wxID_ANY,
    "",
    wxDefaultPosition,
    to_wx(size));
  button->SetBitmap(bmp);
  bind(button, wxEVT_BUTTON, f);
  return button;
}

wxButton* create_button(window_t w,
  const char* label,
  const IntPoint& pos,
  const button_fn& f)
{
  wxButton* button = new wxButton(w.w,
    wxID_ANY,
    label,
    to_wx(pos));
  bind(button, wxEVT_BUTTON, f);
  return button;
}

wxButton* create_button(window_t w,
  const wxBitmap& bmp,
  const button_fn& f)
{
  wxButton* button = new wxButton(w.w,
    wxID_ANY,
    "",
    wxDefaultPosition,
    wxDefaultSize,
    wxBU_EXACTFIT);

  button->SetBitmap(bmp);
  bind(button, wxEVT_BUTTON, f);
  return button;
}

wxButton* create_button(window_t w,
  const wxBitmap& bmp,
  const Tooltip& toolTip,
  const button_fn& f)
{
  wxButton* button = create_button(w,
    bmp, f);
  button->SetToolTip(to_wx(toolTip.Get()));
  return button;
}

wxButton* create_button(window_t w,
  const wxBitmap& bmp,
  const IntSize& size,
  const Tooltip& toolTip,
  const button_fn& f)
{
  wxButton* button = create_button(w,
    bmp, size, f);
  button->SetToolTip(to_wx(toolTip.Get()));
  return button;
}

wxButton* create_ok_button(window_t parent){
  return new wxButton(parent.w, wxID_OK);
}

wxButton* create_cancel_button(window_t parent){
  return new wxButton(parent.w, wxID_CANCEL);
}

wxButton* create_cancel_button(window_t parent, const IntSize& initialSize){
  return new wxButton(parent.w,
    wxID_CANCEL,
    wxEmptyString,
    wxDefaultPosition,
    to_wx(initialSize));
}

wxStaticText* create_label(window_t parent, const char* text){
  return new wxStaticText(parent.w, wxID_ANY, text);
}

wxStaticText* create_label(window_t parent,
  const char* text,
  const IntPoint& pos)
{
  return new wxStaticText(parent.w, wxID_ANY, text, to_wx(pos));
}

wxStaticText* create_label(window_t parent,
  const utf8_string& text,
  FontSize size,
  Bold bold,
  const Color& color)
{
  auto t = new wxStaticText(parent.w, wxID_ANY, to_wx(text));
  t->SetForegroundColour(to_wx(color));
  t->SetFont(wxFontInfo(size.Get()).Bold(bold.Get()));
  return t;
}

wxStaticText* create_label(window_t parent,
  const char* text,
  Bold bold,
  const Color& color)
{
  auto t = new wxStaticText(parent.w, wxID_ANY, text);
  t->SetForegroundColour(to_wx(color));
  t->SetFont(wxFontInfo().Bold(bold.Get()));
  return t;
}

wxStaticText* create_label(window_t parent, const utf8_string& text){
  return new wxStaticText(parent.w, wxID_ANY, to_wx(text));
}

wxTextCtrl* create_text_control(window_t parent, const char* text){
  return new wxTextCtrl(parent.w, wxID_ANY, text);
}

wxTextCtrl* create_text_control(window_t parent, const Width& width){
  return new wxTextCtrl(parent.w, wxID_ANY, "", wxDefaultPosition, to_wx(width));
}

wxTextCtrl* create_text_control(window_t parent,
  const IntPoint& pos,
  const Width& width)
{
  return new wxTextCtrl(parent.w, wxID_ANY, "", to_wx(pos), to_wx(width));
}

wxTextCtrl* create_text_control(window_t parent,
  const char* text,
  const IntPoint& pos,
  const Width& width)
{
  return new wxTextCtrl(parent.w, wxID_ANY,
    text, to_wx(pos), to_wx(width));
}

wxTextCtrl* create_multiline_text_control(window_t parent, const IntSize& size){
  return new wxTextCtrl(parent.w, wxID_ANY, "", wxDefaultPosition,
    to_wx(size),
    wxTE_MULTILINE);
}

void set_focus(window_t w){
  w.w->SetFocus();
}

void set_sizer(window_t w, wxSizer* sizer){
  w.w->SetSizerAndFit(sizer);
}

void set_pos(window_t w, const IntPoint& pos){
  w.w->SetPosition(to_wx(pos));
}

void set_size(window_t w, const IntSize& size){
  w.w->SetSize(to_wx(size));
}

IntPoint get_pos(window_t w){
  return to_faint(w.w->GetPosition());
}

int get_width(window_t w){
  return get_size(w).w;
}

int get_height(window_t w){
  return get_size(w).h;
}

utf8_string get_text(wxTextCtrl* t){
  return to_faint(t->GetValue());
}

utf8_string get_text(wxChoice* c){
  return to_faint(c->GetString(c->GetSelection()));
}

bool select(wxChoice* c, const utf8_string& s){
  int i = c->FindString(to_wx(s));
  if (i == -1){
    return false;
  }
  c->SetSelection(i);
  return true;
}

wxWindow* raw(window_t w){
  return w.w;
}

wxWindow* raw(unique_dialog_ptr& p){
  return p.get();
}

void select_all(wxTextCtrl* t){
  t->SelectAll();
}

wxWindow* create_hline(window_t parent){
  return new wxStaticLine(parent.w, wxID_ANY,
    wxDefaultPosition,
    wxSize(-1, 1),
    wxLI_HORIZONTAL);
}

bool get(wxCheckBox* c){
  return c->GetValue();
}

bool not_set(wxCheckBox* c){
  return !get(c);
}

wxWindow* raw(wxCheckBox* c){
  return c;
}

wxWindow* raw(wxStaticText* txt){
  return txt;
}

void clear(wxCheckBox* c){
  c->SetValue(false);
  wxPostEvent(c, wxCommandEvent(wxEVT_CHECKBOX));
}

bool toggle(wxCheckBox* c){
  bool newValue = !(c->GetValue());
  c->SetValue(newValue);
  wxPostEvent(c, wxCommandEvent(wxEVT_CHECKBOX));
  return newValue;
}

void set(wxCheckBox* c, bool value){
  if (c->GetValue() != value){
    c->SetValue(value);
    wxPostEvent(c, wxCommandEvent(wxEVT_CHECKBOX));
  }
}

wxDialog* center_over_parent(wxDialog* dlg){
  dlg->Center(wxBOTH);
  return dlg;
}

void center_over_parent(unique_dialog_ptr& dlg){
  center_over_parent(dlg.get());
}

void destroy(wxDialog* dlg){
  dlg->Destroy();
}

unique_dialog_ptr null_dialog(){
  return unique_dialog_ptr(nullptr, destroy);
}

void end_modal_ok(wxDialog* dlg){
  dlg->EndModal(wxID_OK);
}

void set_tooltip(window_t w, const Tooltip& tooltip){
  set_tooltip(w, tooltip.Get());
}

void set_tooltip(window_t w, const utf8_string& tooltip){
  w.w->SetToolTip(to_wx(tooltip));
}

wxBookCtrlBase* create_notebook(window_t parent){
  return new wxNotebook(parent.w, wxID_ANY);
}

void add_page(wxBookCtrlBase* book, window_t page, const utf8_string& name){
  book->AddPage(page.w, to_wx(name));
}

void set_selection(wxBookCtrlBase* book, int pageNum){
  book->SetSelection(pageNum);
}

wxWindow* create_hyperlink(window_t parent, const utf8_string url){
  auto urlWx = to_wx(url);
  return new wxHyperlinkCtrl(parent.w, wxID_ANY, urlWx, urlWx);
}

wxWidgetsVersion::wxWidgetsVersion()
  : majorVersion(wxMAJOR_VERSION),
    minorVersion(wxMINOR_VERSION),
    release_number(wxRELEASE_NUMBER),
    subrelease_number(wxSUBRELEASE_NUMBER)
{}

void append_text(wxTextCtrl* textCtrl, const utf8_string& s){
  textCtrl->AppendText(to_wx(s));
}

bool is_empty(wxTextCtrl* textCtrl){
  return textCtrl->IsEmpty();
}

void set_brush(wxDC& dc, const Color& c){
  dc.SetBrush(wxBrush(to_wx(c)));
}

void set_transparent_brush(wxDC& dc){
  wxBrush brush(wxColour(0,0,0));
  brush.SetStyle(wxBRUSHSTYLE_TRANSPARENT);
  dc.SetBrush(brush);
}

void set_pen(wxDC& dc, const Color& c, int w){
  dc.SetPen(wxPen(to_wx(c), w));
}

void set_dotted_pen(wxDC& dc, const Color& c, int w){
  dc.SetPen(wxPen(to_wx(c), w, wxPENSTYLE_DOT));
}

void draw_polygon(wxDC& dc, const std::vector<IntPoint>& points){
  dc.DrawPolygon(static_cast<int>(points.size()), to_wx(points).data()); // Fixme: Check cast
}

void draw_line(wxDC& dc, const IntPoint& p0, const IntPoint& p1){
  dc.DrawLine(p0.x, p0.y, p1.x, p1.y);
}

void set_number_text(wxTextCtrl* ctrl, int value, Signal signal){
  wxString s;
  s << value;
  if (signal == Signal::YES){
    ctrl->SetValue(s);
  }
  else{
    ctrl->ChangeValue(s);
  }
}

void set_bgstyle_paint(window_t w){
  w.w->SetBackgroundStyle(wxBG_STYLE_PAINT);
}

void set_background_color(window_t w, const Color& c){
  w.w->SetBackgroundColour(to_wx(c));
}

Color get_background_color(window_t w){
  return to_faint(w.w->GetBackgroundColour());
}

void set_cursor(window_t w, int cursor){
  w.w->SetCursor(wxCursor(wxStockCursor(cursor)));
}

void refresh(window_t w){
  w.w->Refresh();
}

void process_event(window_t w, wxCommandEvent& e){
  w.w->GetEventHandler()->ProcessEvent(e);
}

} // namespace
