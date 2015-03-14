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

#ifndef FAINT_COLOR_LIST_HH
#define FAINT_COLOR_LIST_HH
#include <vector>
#include "bitmap/color.hh"

namespace faint{

class AlphaMap;
class Bitmap;

class ColorList{
public:
  ColorList();
  void AddColor(const Color&);
  Color GetColor(int index) const;
  int GetNumColors() const;
  auto begin(){
    return m_colors.begin();
  }
  auto end(){
    return m_colors.end();
  }

  auto begin() const{
    return m_colors.begin();
  }
  auto end() const{
    return m_colors.end();
  }
private:
  std::vector<Color> m_colors;
};

Bitmap bitmap_from_indexed_colors(const AlphaMap&, const ColorList&);

} // namespace

#endif
