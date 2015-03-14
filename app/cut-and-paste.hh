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

#ifndef FAINT_CUT_AND_PASTE_HH
#define FAINT_CUT_AND_PASTE_HH

namespace faint{

class AppContext;
class Bitmap;
class Clipboard;
class Paint;

void cut_selection(AppContext&);
void copy_selection(AppContext&);

void paste_to_active_canvas(AppContext&);
void paste_to_new_image(AppContext&);

// Copies the passed in bitmap to the clipboard.
//
// Returns the background color, which is used for alpha replacement
// when pasting outside Faint (if a color), and should be used to
// erase the hole left by a moved selection.
Paint bitmap_to_clipboard(AppContext&, Clipboard&, const Bitmap&);

} // namespace

#endif
