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

#include "wx/window.h" // Fixme
#include "app/resource-id.hh"
#include "geo/int-point.hh"
#include "gui/art.hh"
#include "gui/frame-ctrl.hh"
#include "gui/events.hh" // Fixme: For EVT_FAINT_ControlResized,
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"
#include "util-wx/bind-event.hh"

namespace faint{ class StatusInterface; }
namespace faint{ class DialogContext; }

void gui_test_frame_ctrl(wxWindow* p, faint::StatusInterface& status,
  faint::DialogContext&, const faint::Art& art)
{
  using namespace faint;
  class GuiTestFrameContext : public FrameContext{
  public:
    GuiTestFrameContext()
      : m_numFrames(3),
        m_selectedFrame(0)
    {}

    void AddFrame() override{
      m_numFrames = Index(m_numFrames.Get() + 1);

    }

    void CopyFrame(const OldIndex&, const NewIndex&) override{
      AddFrame();
    }

    Index GetNumFrames() const override{
      return m_numFrames;
    }

    Index GetSelectedFrame() const override{
      return m_selectedFrame;
    }

    void MoveFrame(const OldIndex&, const NewIndex&) override{}

    void RemoveFrame(const Index&) override{
      if (m_numFrames == 1){
        return;
      }
      m_numFrames = Index(m_numFrames.Get() - 1);
      if (m_numFrames <= m_selectedFrame){
        m_selectedFrame = Index(m_numFrames.Get() - 1);
      }
    }

    void SelectFrame(const Index& i) override{
      m_selectedFrame = i;
    }

    Index m_numFrames;
    Index m_selectedFrame;
  };

  auto f = new FrameCtrl(p,
    std::make_unique<GuiTestFrameContext>(),
    status,
    art);

  using namespace layout;
  set_sizer(p,
    create_row({grow(f->AsWindow())}));

  f->Update();
  refresh_layout(p);
  bind(p, EVT_FAINT_ControlResized,
    [=](){
      refresh_layout(p);
      refresh(p);
    });
}
