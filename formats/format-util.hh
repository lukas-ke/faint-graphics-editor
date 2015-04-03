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

#ifndef FAINT_FORMAT_UTIL_HH
#define FAINT_FORMAT_UTIL_HH

#include "bitmap/bitmap.hh"
#include "util/image-props.hh"
#include "util/or-error.hh"

namespace faint{

// Adds the bitmap as a frame to the ImageProps, or sets the error to
// the ImageProps.
void add_frame_or_set_error(OrError<Bitmap>&&, ImageProps&) ;

} // namespace

#endif
