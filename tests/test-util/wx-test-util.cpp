// -*- coding: us-ascii-unix -*-
#include "wx/bitmap.h"
#include "wx/rawbmp.h"
#include "wx/dcmemory.h"
#include "bitmap/color.hh"
#include "tests/test-util/wx-test-util.hh"
#include "util-wx/convert-wx.hh"

namespace faint{

using PixelData = wxPixelData<wxBitmap, wxAlphaPixelFormat>;
Color get_color_wxBitmap(wxBitmap& bmp, int x, int y){
  PixelData pd(bmp);
  assert(pd);
  PixelData::Iterator p = pd;
  p.OffsetY(pd, y);
  for (int i = 0; i != x; i++){
    p++;
  }

  return Color(p.Red(), p.Green(), p.Blue(), p.Alpha());
}

} // namespace
