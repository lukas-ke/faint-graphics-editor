#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2012 Lukas Kemmer
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You
# may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

from faint.pdf.document import Document
from faint.pdf.stream import Stream
from faint.pdf.convert_to_pdf import object_to_stream
from faint.pdf.xobject import XObject
from faint.formatutil import open_for_writing_binary
import ifaint

__all__ = ("write",)

def _add_page(doc, frame):
    """Adds the Faint frame (and all objects) as a page to the
    document"""
    frame_size = frame.get_size()
    page_width, page_height = frame_size
    scale_x = frame_size[0] / page_width
    scale_y = frame_size[1] / page_height

    stream = Stream()
    meta = []

    if not ifaint.one_color_bg(frame):
        bmp = frame.get_bitmap()
        w, h = bmp.get_size()
        bg_name = doc.add_xobject(XObject(bmp.get_raw_rgb_string(), w, h))
        stream.xobject(bg_name, 0, 0, page_width, page_height)

    for obj in frame.get_objects():
        obj_meta = object_to_stream(stream, obj, page_height, scale_x, scale_y)
        meta.extend(obj_meta)

    page_id = doc.add_page(page_width, page_height)
    doc.add_comment("faint meta-data")
    for entry in meta:
        doc.add_comment(entry)
    doc.add_comment("end of faint meta-data")

    doc.add_stream(stream, page_id)


def write(file_path, canvas):
    """Write the canvas as PDF to the specified file"""

    doc = Document()
    for frame in canvas.get_frames():
        _add_page(doc, frame)

    f = open_for_writing_binary(file_path)
    try:
        f.write(str(doc).encode("ascii")) # Fixme: ascii will fail for values > 127
    finally:
        f.close()
