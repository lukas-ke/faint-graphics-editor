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

#include "text/utf8-string.hh"
#include "util-wx/clipboard.hh"
#include "util/optional.hh"
#include "python/py-clipboard.hh"
#include "python/py-function-error.hh"
#include "python/py-bitmap.hh"
#include "python/py-ugly-forward.hh"
#include "python/py-parse.hh"

namespace faint{

inline OSError failed_open_clipboard(){
  return OSError("Failed opening clipboard");
}

static void copy_bitmap_with_bgcolor(const Bitmap& bmp, const ColRGB& bg){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw failed_open_clipboard();
  }
  clipboard.SetBitmap(bmp, bg);
}

static void copy_bitmap(const Bitmap& bmp){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw failed_open_clipboard();
  }
  clipboard.SetBitmap(bmp, ColRGB(255,255,255));
}

static void copy_text(const utf8_string& s){
  Clipboard clipboard;
  if (!clipboard.Good()){
    throw failed_open_clipboard();
  }
  clipboard.SetText(s);
}

/* function: "set(bmp[,(r,g,b)])\n
Copy the bitmap to the clipboard. When rgb is specified, pixels with
alpha will be blended onto this background when pasting outside Faint.\n\n

set(txt)\n
Copy the text to the clipboard."
name: "set" */
static void clipboard_set(PyObject* args){
  if (PySequence_Length(args) == 1){
    try{
      FORWARD_PY(copy_bitmap)(nullptr, args);
    }
    catch(const TypeError&){
      FORWARD_PY(copy_text)(nullptr, args);
    }
  }
  else{
    FORWARD_PY(copy_bitmap_with_bgcolor)(nullptr, args);
  }
}

/* function: "get_text() -> s?\n
Returns the text from the clipboard as a str or None if no text is
available." */
static Optional<utf8_string> get_text(){
  Clipboard c;
  if (!c.Good()){
    throw failed_open_clipboard();
  }

  return c.GetText();
}

/* function: "get_bitmap() -> bmp?\n
Returns the bitmap from the clipboard or None if no bitmap is
available." */
static Optional<Bitmap> get_bitmap(){
  Clipboard c;
  if (!c.Good()){
    throw failed_open_clipboard();
  }

  return c.GetBitmap();
}

#include "generated/python/method-def/py-clipboard-methoddef.hh"

static struct PyModuleDef faintClipboardModule {
  PyModuleDef_HEAD_INIT,
  "clipboard",
  "clipboard\n\nFunctions for accessing the clipboard for copying "
  "bitmaps and text.",
  -1, // m_size
  clipboard_methods, // m_methods
  nullptr, // m_reload
  nullptr, // m_traverse
  nullptr, // m_clear
  nullptr, // m_free
};

PyObject* create_clipboard_module(){
  PyObject* module_clipboard = PyModule_Create(&faintClipboardModule);
  assert(module_clipboard != nullptr);
  return module_clipboard;
}

} // namespace
