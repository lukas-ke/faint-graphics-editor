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
#include "wx/button.h"
#include "wx/dialog.h"
#include "wx/frame.h"
#include "wx/filedlg.h"
#include "wx/filename.h"
#include "wx/msgdlg.h"
#include "wx/persist.h"
#include "wx/textctrl.h"
#include "wx/persist/toplevel.h"
#include "app/app-context.hh"
#include "gui/accelerator-entry.hh"
#include "gui/art.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/gui-util.hh"
#include "util/image-props.hh"
#include "util/iter.hh"

namespace faint{

void console_message(const utf8_string& text){
  wxMessageOutput* msgOut = wxMessageOutput::Get();
  assert(msgOut != nullptr);
  msgOut->Output(to_wx(text));
}

static int show_modal(wxDialog& dlg, AppContext& app){
  // Application::FilterEvent catches keypresses even when
  // dialogs are shown. Using this function for all dialogs
  // shown by the frame allows handling it.
  app.BeginModalDialog();
  int result = dlg.ShowModal();
  app.EndModalDialog();
  return result;
}

wxColour get_gui_selected_color(){
  return {112, 154, 209};
}

wxColour get_gui_deselected_color(){
  return {255, 255, 255};
}

wxIcon get_icon(const Art& art, Icon iconId){
  wxIcon icon;
  icon.CopyFromBitmap(art.Get(iconId));
  return icon;
}

wxIconBundle bundle_icons(const wxIcon& icon1, const wxIcon& icon2){
  wxIconBundle bundle;
  bundle.AddIcon(icon1);
  bundle.AddIcon(icon2);
  return bundle;
}

void fit_size_to(wxTextCtrl* textCtrl, const utf8_string& str){
  wxSize extents = textCtrl->GetTextExtent(to_wx(str));
  textCtrl->SetInitialSize(wxSize(extents.x + 10, -1));
}

void restore_persisted_state(wxFrame* frame, const storage_name& name){
  frame->SetName(to_wx(name.Get()));
  wxPersistenceManager::Get().RegisterAndRestore(frame);
}

int parse_int_value(wxTextCtrl* textCtrl, int defaultValue){
  std::string s(textCtrl->GetValue());
  std::stringstream ss(s);
  int i = defaultValue;
  ss >> i;
  return i;
}

coord parse_coord_value(wxTextCtrl* textCtrl, coord defaultValue){
  std::string s(textCtrl->GetValue());
  std::stringstream ss(s);
  coord v = defaultValue;
  ss >> v;
  return v;
}

bool ask_exit_unsaved_changes(wxWindow* parent, AppContext& app){
  wxMessageDialog dlg(parent,
    "One or more files have unsaved changes.\nExit anyway?",
    "Unsaved Changes", wxYES_NO | wxNO_DEFAULT | wxICON_WARNING);
  const int choice = show_modal(dlg, app);
  return choice == wxID_YES;
}

static wxString get_save_target_string(const FilePath& path){
  return wxString("'") + to_wx(path.Str()) + "'";
}

SaveChoice ask_close_unsaved_tab(wxWindow* parent, AppContext& app,
  const Optional<FilePath>& filePath)
{
  const wxString title = "Unsaved Changes";
  const wxString message = "Save changes to " +
    (filePath.IsSet() ? get_save_target_string(filePath.Get()) :
      get_new_canvas_title()) + "?";

  const long flags = static_cast<long>(wxYES_NO|wxCANCEL|wxCANCEL_DEFAULT);
  wxMessageDialog dlg(parent, message, "Unsaved Changes", flags);
  int choice = show_modal(dlg, app);
  assert(choice == wxID_YES || choice == wxID_NO || choice == wxID_CANCEL);
  return static_cast<SaveChoice>(choice);
}

wxString get_new_canvas_title(){
  return "Untitled";
}

void show_copy_color_error(wxWindow* parent, AppContext& app){
  wxMessageDialog dlg(parent,
    "Failed Copying the color.\n\nThe clipboard could not be opened.",
    "Copy Color Failed",
    wxOK|wxICON_ERROR);
  show_modal(dlg, app);
}

void show_error(wxWindow* parent,
  AppContext& app,
  const Title& title,
  const utf8_string& message)
{
  wxMessageDialog dlg(parent,
    to_wx(message),
    to_wx(title.Get()),
    wxOK|wxICON_ERROR);
  show_modal(dlg, app);
}

void show_error(wxWindow& parent,
  AppContext& app,
  const Title& title,
  const utf8_string& message)
{
  show_error(&parent, app, title, message);
}

void show_error_from_dialog(wxWindow& parent,
  const Title& title,
  const utf8_string& message)
{
  wxMessageDialog dlg(&parent,
    to_wx(message),
    to_wx(title.Get()),
    wxOK|wxICON_ERROR);
  dlg.ShowModal();
}

bool show_init_error(const Title& title, const utf8_string& message){
  wxMessageDialog dlg(nullptr,
    to_wx(message),
    to_wx(title.Get()),
    wxOK|wxICON_ERROR);
  dlg.ShowModal();
  return false;
}

static void show_warning(wxWindow* parent,
  AppContext& app,
  const Title& title,
  const utf8_string& message)
{
  wxMessageDialog dlg(parent,
    to_wx(message),
    to_wx(title.Get()),
    wxOK|wxICON_WARNING);
  show_modal(dlg, app);
}

void show_file_not_found_error(wxWindow* parent,
  AppContext& app,
  const FilePath& file)
{
  wxString errStr("File not found:\n");
  errStr += to_wx(file.Str());
  wxMessageDialog dlg(parent, errStr, "File not found", wxOK|wxICON_ERROR);
  show_modal(dlg, app);
}

void show_file_not_supported_error(wxWindow* parent,
  AppContext& app,
  const FilePath& file)
{
  wxString errStr("File type not supported:\n");
  errStr += to_wx(file.Str());
  wxMessageDialog dlg(parent, errStr, "Unsupported File", wxOK|wxICON_ERROR);
  show_modal(dlg, app);
}

void show_load_failed_error(wxWindow* parent,
  AppContext& app,
  const FilePath& file,
  const utf8_string& message)
{
  wxString errStr("Failed loading:\n");
  errStr += to_wx(file.Str());
  errStr += "\n\n";
  errStr += to_wx(message);
  wxMessageDialog dlg(parent, errStr, "Failed Loading Image", wxOK|wxICON_ERROR);
  show_modal(dlg, app);
}

void show_load_warnings(wxWindow* parent,
  AppContext& app,
  const ImageProps& props)
{
  utf8_string warnings;
  for (int i = 0; i != props.GetNumWarnings() && i < 10; i++){
    warnings += (props.GetWarning(i) + "\n");
  }
  show_warning(parent, app, Title("Warning"), warnings);
}

FileList show_open_file_dialog(wxWindow& parent,
  AppContext& app,
  const Title& title,
  const Optional<DirPath>& initialPath,
  const utf8_string& filter)
{
  wxFileDialog fd(&parent,
    to_wx(title.Get()),
    initialPath.IsSet() ? to_wx(initialPath.Get().Str()) : wxString(""),
    wxString(""), // Default file
    to_wx(filter),
    wxFD_OPEN | wxFD_MULTIPLE | wxFD_CHANGE_DIR | wxFD_FILE_MUST_EXIST);
  if (show_modal(fd, app) == wxID_OK){
    wxArrayString paths;
    fd.GetPaths(paths);
    return to_FileList(paths);
  }
  return {};
}

void show_out_of_memory_cancelled_error(wxWindow* parent, AppContext& app){
  show_error(parent, app, Title("Out of memory"),
    "An action required too much memory and was cancelled.");
}

static int accel_from_key(const KeyPress& keyPress){
  const Mod mod = keyPress.Modifiers();
  int flags = wxACCEL_NORMAL;
  if (mod.Alt()){
    flags |= wxACCEL_ALT;
  }
  if (mod.Ctrl()){
    flags |= wxACCEL_CTRL;
  }
  if (mod.Shift()){
    flags |= wxACCEL_SHIFT;
  }
  return flags;
}


void set_accelerators(wxWindow* window,
  const std::vector<AcceleratorEntry>& entries)
{
  std::vector<wxAcceleratorEntry> wx_entries;

  int num = 0;
  for (const auto& entry : entries){
    for (const auto& keyPress : entry.keyPresses){
      const int cmdId = num++;
      wx_entries.push_back(wxAcceleratorEntry(accel_from_key(keyPress),
        keyPress.GetKeyCode(), cmdId));
      window->Bind(wxEVT_COMMAND_MENU_SELECTED,
        [func = entry.func](wxCommandEvent&){
          func();
        }, cmdId);
    }
  }

  window->SetAcceleratorTable(wxAcceleratorTable(resigned(wx_entries.size()),
    wx_entries.data()));
}

} // namespace
