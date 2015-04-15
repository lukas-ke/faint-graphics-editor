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

#ifndef FAINT_PNG_UTIL_HH
#define FAINT_PNG_UTIL_HH

namespace faint{

inline bool gray_or_gray_alpha(int colorType){
  return colorType == PNG_COLOR_TYPE_GRAY ||
    colorType == PNG_COLOR_TYPE_GRAY_ALPHA;
}

inline bool rgb_or_rgba(int colorType){
  return colorType == PNG_COLOR_TYPE_RGB ||
    colorType == PNG_COLOR_TYPE_RGB_ALPHA;
}

inline bool palettized(int colorType){
  return colorType == PNG_COLOR_TYPE_PALETTE;
}

} // namespace

#endif
