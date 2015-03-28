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

#include "formats/format-util.hh"

namespace faint{

void add_frame_or_set_error(OrError<Bitmap>&& obj, ImageProps& props){
  obj.Visit(
    [&](Bitmap& bmp){
      props.AddFrame(std::move(bmp), FrameInfo());
    },
    [&](const utf8_string& s){
      props.SetError(s);
    });
}

} // namespace
