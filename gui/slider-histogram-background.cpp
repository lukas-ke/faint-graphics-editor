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

#include <algorithm>
#include "bitmap/draw.hh"
#include "bitmap/histogram.hh"
#include "geo/int-rect.hh"
#include "geo/primitive.hh"
#include "gui/slider-histogram-background.hh"
#include "util-wx/system-colors.hh"

namespace faint{

SliderHistogramBackground::SliderHistogramBackground(
  const std::vector<int>& values,
  const ColRGB& fg)
  : m_bitmap(),
    m_values(values),
    m_fg(fg)
{}

void SliderHistogramBackground::Draw(Bitmap& bmp, const IntSize& size, SliderDir){
  if (!bitmap_ok(m_bitmap) || m_bitmap.GetSize() != size){
    InitializeBitmap(size);
  }
  blit(at_top_left(m_bitmap), onto(bmp));
}

SliderBackground* SliderHistogramBackground::Clone() const {
  return new SliderHistogramBackground(*this);
}

void SliderHistogramBackground::InitializeBitmap(const IntSize& size){
  const int numValues = resigned(m_values.size());
  const bin_t bins(std::min(numValues, size.w));
  Histogram histogram(ClosedIntRange(min_t(0), max_t(numValues)), bins);
  for (int i = 0; i != numValues; i++){
    histogram.Insert(i, count_t(m_values[to_size_t(i)]));
  }
  int max = histogram.Max();

  // Pixels per bin
  double binWidth = std::max(size.w / double(histogram.NumBins()), 1.0);

  double scale_h = size.h / double(max);
  m_bitmap = Bitmap(size, color_button_face());

  for (int bin = 0; bin < histogram.NumBins(); bin++){
    int x = static_cast<int>(bin * binWidth);
    int y = static_cast<int>(size.h - histogram.Count(bin_t(bin)) * scale_h);
    fill_rect_color(m_bitmap, IntRect(IntPoint(x,y),
        IntSize(int(binWidth) + 1, size.h)), Color(m_fg,255));
  }

  draw_sunken_ui_border(m_bitmap, {IntPoint(0,0), size});

}

} // namespace
