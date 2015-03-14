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

#include "bitmap/bitmap.hh"
#include "bitmap/color.hh"
#include "bitmap/paint.hh"
#include "geo/int-point.hh"
#include "geo/int-rect.hh"
#include "gui/dialogs.hh"
#include "gui/resize-dialog.hh"
#include "text/formatting.hh"
#include "util/optional.hh"
#include "util-wx/fwd-wx.hh"
#include "util-wx/layout-wx.hh"

namespace faint{ class StatusInterface; }

namespace{
using namespace faint;
class TestDialogFeedback : public DialogFeedback{
public:
  TestDialogFeedback()
    : m_bitmap({640,480}, color_white)
  {}

  const Bitmap& GetBitmap() override{
    return m_bitmap;
  }

  void SetBitmap(const Bitmap&) override{}
  void SetBitmap(Bitmap&&) override{}

private:
  Bitmap m_bitmap;
};

TestDialogFeedback g_testFeedback;

} // namespace


void gui_test_alpha_dialog(wxWindow* p, faint::StatusInterface&,
  faint::DialogContext& dialogContext)
{
  using namespace faint;

  set_sizer(p,
    layout::create_column({
      create_button(p, "Alpha dialog",
        [&dialogContext](){
          show_alpha_dialog(dialogContext);
        })}));
}
