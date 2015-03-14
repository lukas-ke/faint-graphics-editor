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

#ifndef FAINT_IMAGE_UTIL_HH
#define FAINT_IMAGE_UTIL_HH
#include "util/common-fwd.hh"

namespace faint{

// True if the specified objects can be moved backward in the Image
bool can_move_backward(const Image&, const objects_t&);

// True if the specified objects can be moved forward in the Image
bool can_move_forward(const Image&, const objects_t&);

// True if the image has all the specified objects
bool has_all(const Image&, const objects_t&);

// True if the point is a valid pixel position in the image
bool point_in_image(const Image&, const IntPoint&);

// Gets a subbitmap from the Image. Creates a bitmap on its own
// accord if the image has no raster background.
Bitmap subbitmap(const Image&, const IntRect&);

// Returns the background from the image with the selection
// stamped onto it. The bitmap will not contain the objects.
//
// Use this function to avoid losing the floating raster selection
// graphic when saving to vector formats with a raster background.
Bitmap stamp_raster_selection(const Image&);

// Returns a bitmap with the background from the image with all
// objects and any floating selection stamped onto it.
// Use to include objects and the floating raster selection graphic
// when saving to raster formats.
Bitmap flatten(const Image&);

// Gets the highest Z-value in the image (the front-most object).
// Asserts that the image has objects.
int get_highest_z(const Image&);

// Removes objects that are not in the image from the container
void remove_missing_objects_from(objects_t&, const Image&);

IntRect image_rect(const Image&);

// Returns the pixel to mm conversion for the image, if calibrated
// in a unit related to mm
Optional<coord> get_pixel_to_mm_conversion(const Image&);

} // namespace

#endif
