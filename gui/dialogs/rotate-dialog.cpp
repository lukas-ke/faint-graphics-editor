// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#include <functional>
#include "wx/dialog.h"
#include "wx/panel.h"
#include "app/canvas.hh"
#include "app/context-commands.hh"
#include "app/resource-id.hh"
#include "geo/int-point.hh"
#include "geo/measure.hh"
#include "gui/art.hh"
#include "gui/dialog-context.hh"
#include "gui/ui-constants.hh"
#include "text/formatting.hh"
#include "tools/tool-id.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/gui-util.hh"
#include "util-wx/key-codes.hh"
#include "util-wx/layout-wx.hh"
#include "util-wx/placement.hh"
#include "util/apply-target.hh"
#include "util/command-util.hh"
#include "util/object-util.hh"

namespace faint{

using Accelerators = std::vector<AcceleratorEntry>;

static CommandPtr dummy_function(const Canvas&){
  // Avoids leaving RotateDialog::m_cmdFunc uninitialized
  return nullptr;
}

using cmd_func = std::function<decltype(dummy_function)>;
using cmd_event_func = accel_fn;

static wxString get_rotate_target_name(const Canvas& canvas){
  return dispatch_target(get_apply_target(canvas),
    [&](OBJECT_SELECTION){
      return to_wx(get_collective_type(canvas.GetObjectSelection()));
    },
    [](RASTER_SELECTION){
      return wxString("Selection");
    },
    [](IMAGE){
      return wxString("Image");
    });
}

class RotateChoicePanel : public wxPanel{
  // The panel shown initially in the rotate dialog

public:
  RotateChoicePanel(wxWindow* parent,
    const Art& art,
    cmd_event_func horizontal_flip,
    cmd_event_func vertical_flip,
    cmd_event_func rotate,
    cmd_event_func level)
    : wxPanel(parent, wxID_ANY),
      m_cancelButton(nullptr),
      m_rotateButton(nullptr)
  {
    Accelerators acc;
    using namespace layout;
    set_sizer(this,
      create_column({
        create_row_outer_pad({
          CreateButton(Tooltip("Flip Horizontally"),
            art.Get(Icon::ROTATE_DIALOG_FLIP_HORIZONTAL),
            acc, key::Q,
            horizontal_flip),

          CreateButton(Tooltip("Flip Vertically"),
            art.Get(Icon::ROTATE_DIALOG_FLIP_VERTICAL),
            acc, key::W,
            vertical_flip),

          m_rotateButton = CreateButton(Tooltip("Rotation..."),
              art.Get(Icon::ROTATE_DIALOG_ROTATE_NEXT),
              acc, key::E,
              rotate),

          m_cancelButton = raw(create_cancel_button(this, ui::big_button_size))}),

        CreateLevelRow(art, acc, level)}));
    set_accelerators(this, (acc));
  }

  wxSizer* CreateLevelRow(const Art& art,
    Accelerators& acc,
    cmd_event_func select_level_tool)
  {
    wxStaticText* label = create_label(this, "&Horizontal level");
    wxWindow* levelButton = raw(create_button(window_t(this),
      art.Get(Icon::TOOL_LEVEL),
      Tooltip("Horizontal level tool"),
      select_level_tool));

    acc.push_back({key::H, Alt+key::H, select_level_tool});

    using namespace layout;
    return create_row_outer_pad({levelButton,
      {raw(label), Proportion(0), wxALIGN_CENTER_VERTICAL}});
  }

  wxWindow* CreateButton(const Tooltip& tooltip,
    const wxBitmap& bmp,
    Accelerators& acc,
    const Key& keyCode,
    cmd_event_func cmdEvtFunc)
  {
    acc.push_back({keyCode, cmdEvtFunc});
    return raw(create_button(this,
      bmp,
      ui::big_button_size,
      tooltip,
      cmdEvtFunc));

  }

  IntPoint GetRotateButtonPos() const{
    return get_pos(m_rotateButton);
  }

  IntPoint GetCancelButtonPos() const{
    return get_pos(m_cancelButton);
  }

  Key GetRotateAccelerator() const{
    return key::E;
  }
private:
  wxWindow* m_cancelButton;
  wxWindow* m_rotateButton;
};

class AngleChoicePanel : public wxPanel{
public:
  AngleChoicePanel(wxWindow* parent,
    const Art& art,
    const RotateChoicePanel& siblingPanel,
    cmd_event_func rotate)
    : wxPanel(parent, wxID_ANY),
      m_angleTextCtrl(nullptr)
  {
    SetSize(siblingPanel.GetSize());
    Accelerators acc;
    CreateEntryRow(art, siblingPanel, acc, rotate);
    set_accelerators(this, acc);
  }

  bool Modified() const{
    return get_text(m_angleTextCtrl) != "90";
  }

  Angle GetAngle() const{
    return Angle::Deg(parse_int_value(m_angleTextCtrl, 90));
  }

  void CreateEntryRow(const Art& art,
    const RotateChoicePanel& siblingPanel,
    Accelerators& acc,
    cmd_event_func rotate)
  {
    wxStaticText* label = create_label(this, "&Degrees");
    m_angleTextCtrl = create_text_control(this, "90");
    fit_size_to(m_angleTextCtrl, "9999999");

    m_rotateBtn = create_button(this,
      art.Get(Icon::ROTATE_DIALOG_ROTATE),
      ui::big_button_size,
      rotate);
    auto rotateKey = siblingPanel.GetRotateAccelerator();
    acc.push_back({rotateKey, rotate});

    set_pos(m_rotateBtn, siblingPanel.GetRotateButtonPos());

    wxButton* cancelButton = create_cancel_button(this, ui::big_button_size);
    set_pos(cancelButton, siblingPanel.GetCancelButtonPos());

    set_pos(m_angleTextCtrl, to_the_left_middle_of(raw(m_rotateBtn),
      get_size(m_angleTextCtrl)));
    set_pos(label, to_the_left_middle_of(raw(m_angleTextCtrl), get_size(label)));
  }

  void FocusEntry(){
    set_focus(m_rotateBtn);
    set_focus(m_angleTextCtrl);
    select_all(m_angleTextCtrl);
  }

private:
  wxTextCtrl* m_angleTextCtrl;
  wxButton* m_rotateBtn;
};

class RotateDialog : public wxDialog{
public:
  RotateDialog(wxWindow& parent,
    wxString targetName,
    const Art& art,
    const std::function<Paint()>& getBg,
    const std::function<void(ToolId)>& selectTool)
    : wxDialog(&parent, wxID_ANY, "Flip/Rotate " + targetName),
      m_cmdFunc(dummy_function),
      m_firstPanel(nullptr),
      m_getBg(getBg),
      m_secondPanel(nullptr),
      m_selectTool(selectTool),
      m_targetName(targetName)
  {
    m_firstPanel = new RotateChoicePanel(this, art,
      [=](){SetFunctionAndClose(&context_flip_horizontal);},
      [=](){SetFunctionAndClose(&context_flip_vertical);},
      [=](){SwitchPanel();},
      [=](){SelectLevelTool();});

    m_secondPanel = new AngleChoicePanel(this, art, *m_firstPanel,
      [=](){RotateAndClose();});
    m_secondPanel->Hide();

    Fit(); // Shrink the dialog to the panel size

    center_over_parent(this);
  }

  CommandPtr GetCommand(const Canvas& canvas){
    return m_cmdFunc(canvas);
  }

private:

  void SetFunctionAndClose(cmd_func func){
    m_cmdFunc = func;
    EndModal(wxID_OK);
  }

  void RotateAndClose(){
    m_cmdFunc = [=](const Canvas& canvas){
      return
        m_secondPanel->Modified() ?
          context_rotate(canvas,
            m_secondPanel->GetAngle(),
            m_getBg()) :
          context_rotate90cw(canvas);
    };
    EndModal(wxID_OK);
  }

  void SelectLevelTool(){
    m_cmdFunc = nullptr;
    m_selectTool(ToolId::LEVEL);
    EndModal(wxID_CANCEL);
  }

  void SwitchPanel(){
    m_firstPanel->Hide();
    m_secondPanel->Show();
    m_secondPanel->FocusEntry();
    SetTitle("Rotate " + m_targetName);

    // Deleting the first panel instead of disabling it, so that the
    // escape-key targets the cancel button in the second panel
    // instead of a disabled cancel-button in the first panel.
    delete m_firstPanel;
    m_firstPanel = nullptr;
  }

  // The function that creates a Command. Depends on the user choice.
  cmd_func m_cmdFunc;
  RotateChoicePanel* m_firstPanel;
  std::function<Paint()> m_getBg;
  AngleChoicePanel* m_secondPanel;
  std::function<void(ToolId)> m_selectTool;
  wxString m_targetName;
};


static Optional<CommandPtr> show_rotate_dialog(wxWindow& parent,
  const Canvas& canvas,
  const Art& art,
  const std::function<Paint()>& bgColor,
  const std::function<void(ToolId)>& selectTool,
  DialogContext& c)
{
  RotateDialog dlg(parent,
    get_rotate_target_name(canvas),
    art,
    bgColor,
    selectTool);

  if (c.ShowModal(dlg) == DialogChoice::OK){
    return Optional<CommandPtr>(dlg.GetCommand(canvas));
  }
  else{
    return {};
  }
}

dialog_func bind_show_rotate_dialog(const Art& art,
  const std::function<Paint()>& getBgColor,
  const std::function<void(ToolId)>& selectTool,
  DialogContext& dialogContext)
{
  return [&, getBgColor, selectTool]
    (wxWindow& window, DialogFeedback&, const Canvas& canvas){
    return show_rotate_dialog(window,
      canvas,
      art,
      getBgColor,
      selectTool,
      dialogContext);
  };
}

} // namespace
