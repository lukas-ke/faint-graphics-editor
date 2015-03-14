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

#ifndef FAINT_CLIPBOARD_HH
#define FAINT_CLIPBOARD_HH
#include "util/objects.hh"
#include "util/template-fwd.hh"

namespace faint {

class Bitmap;
class ColRGB;
class utf8_string;

class Clipboard{
public:
  Clipboard();
  ~Clipboard();
  static void Flush();
  Optional<Bitmap> GetBitmap();
  Optional<objects_t> GetObjects();
  Optional<utf8_string> GetText();

  bool Good() const;

  // Puts the bitmap in the clipboard. When pasted outside Faint,
  // pixels with alpha will be blended onto bgCol.
  void SetBitmap(const Bitmap&, const ColRGB& bgCol);
  void SetObjects(const objects_t&);
  void SetText(const utf8_string&);
private:
  bool m_ok;
};

} // namespace

#endif
