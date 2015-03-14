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

import codecs
import re
import ifaint
from ifaint import Settings
from faint.pdf.util import unescape_string

__all__ = ("parse",)

def _find_page_sizes(text):
    """Finds page sizes by the MediaBox attribute.
    Returns a list of width,height-tuples.
    Hack - this assumes that only pages have a MediaBox attribute.
    """
    re_mediabox = re.compile(r"/MediaBox \[(\d+ \d+ \d+ \d+)\]")
    sizes = []
    for m in re_mediabox.finditer(text):
        x,y,w,h = [int(item) for item in m.group(1).split(" ")]
        sizes.append((w,h))
    return sizes


def _find_streams(text):
    """Finds data streams in text, returns a list of strings containing
    the stream contents"""
    re_stream = re.compile(r"<< /Length \d+ >>\n(stream.*?endstream)", re.DOTALL)
    streams = []
    for m in re_stream.finditer(text):
        streams.append(text[m.start(1):m.end(1)])
    return streams


def _from_pdf_color(r, g, b):
    return int(r * 255), int(g * 255), int(b * 255)


def _parse_metadata(metadata):
    """Parse a Faint meta-data comment."""
    d = {}
    for line in metadata.strip().split('\n'):
        lineNum, data = line[2:].split(":")
        d[int(lineNum)] = data.split(" ")
    return d

def _parse_text(items):
    return unescape_string(" ".join(items[0:-2])[1:])

def _parse_stream(text, frame_props, page_size, metadata):
    """Parses a pdf-stream object into Faint objects which are added
    to the props."""
    doc_h = page_size[1]
    lines = text.split("\n")
    assert(lines[0] == "stream")
    pos_x, pos_y = 0, 0
    points = []
    settings = Settings()
    settings.linewidth = 1
    settings.linestyle ='s'
    settings.fg = 0,0,0 # Fixme
    settings.bg = 0,0,0 # Fixme
    closed = False
    arrowHead = False
    skip = 0
    arrowEndX = 0 # Fixme
    arrowEndY = 0 # Fixme

    # Fixme: Messy code ahead
    curr_text_pos = 0,0
    for num, line in enumerate(lines):
        if skip > 0:
            # The meta-data can request skipping stream lines (This
            # allows pdf viewers to render e.g. arrow heads as
            # polygons, while Faint should load the arrow head back
            # using the meta data, and ignore the polygon)
            skip -= 1
            continue
        if num in metadata:
            meta = metadata[num]
            if meta[0] == "faint-arrow":
                # Ignore the lines that constitute the arrowhead
                skip = 3
                settings.arrow = 'front'
                arrowHead = True
                arrowEndX = float(meta[1])
                arrowEndY = float(meta[2])
                continue
            elif meta[0] == "faint-ellipse":
                x, y, w, h = [float(v) for v in meta[1:5]]
                p = frame_props.Ellipse((x,y,w,h), settings)
                skip = 6
                continue

        items = line.split(" ")
        if items[-1] == 'm':
            points.extend((float(items[0]), float(items[1])))
        elif items[-1] == 'l':
            points.extend([float(item) for item in items[:-1]])

        elif items[-1] == 'S' or items[-1] == "B": # Fixme: stroke/fill etc.
            if len(points) == 4:
                if arrowHead:
                    points[-2] = arrowEndX
                    points[-1] = arrowEndY
                p = frame_props.Line(_points_pdf_to_faint(points, doc_h, 1.0, 1.0), settings)
            elif len(points) > 4:
                if closed:
                    if items[-1] == "B":
                        settings.fillstyle = 'bf'
                    p = frame_props.Polygon(_points_pdf_to_faint(points, doc_h, 1.0, 1.0), settings)
                else:
                    p = frame_props.Line(_points_pdf_to_faint(points, doc_h, 1.0, 1.0) , settings)
            closed = False
            points = []
            settings.linewidth = 1
            settings.linestyle = 's'
            settings.fillstyle = 'b'
            settings.arrow = 'none'
        elif items[0] == 'f*':
            if len(points) > 4:
                settings.fillstyle = 'f'
                p = frame_props.Polygon(_points_pdf_to_faint(points, doc_h, 1.0, 1.0), settings)
            points = []
        elif items[-1] == 'w':
            settings.linewidth = float(items[0])
        elif items[-1] == 'd':
            if items[0] == "[]":
                settings.linestyle = "s"
            else:
                settings.linestyle = "ld"
        elif items[0] == 'h':
            closed = True
        elif items[-1] == 'RG':
            settings.fg = _from_pdf_color(*[float(item) for item in items[:-1]])
        elif items[-1] == 'rg':
            settings.bg = _from_pdf_color(*[float(item) for item in items[:-1]])
        elif items[-1] == 'f':
            settings.fillstyle = 'f'
            settings.fg, settings.bg = settings.bg, settings.fg
            p = frame_props.Polygon(_points_pdf_to_faint(points, doc_h, 1.0, 1.0), settings)

        elif items[-1] == 'Td':
            curr_text_pos = float(items[0]), float(items[1])

        elif items[-1] == 'Tj':
            p = _points_pdf_to_faint(curr_text_pos, doc_h, 1.0, 1.0)
            text = _parse_text(items)
            t = frame_props.Text((p[0], p[1], 100, 100), text, settings)
        else:
            pass


def _points_pdf_to_faint(pdf_points, doc_h, scale_x, scale_y):
    faint_points = []
    for i in range(len(pdf_points)):
        if i % 2 == 0:
            faint_points.append(pdf_points[i] / scale_x)
        else:
            faint_points.append((doc_h - pdf_points[i]) / scale_y)
    return faint_points


def _find_faint_meta(text):
    re_metadata = re.compile("% faint meta-data\n(.*?)% end of faint meta-data", re.DOTALL)
    meta_data = [] # List of meta-data dictionaries
    for match in re_metadata.finditer(text):
        if len(match.group(1)) > 0:
            meta_data.append(_parse_metadata(match.group(1)))
    return meta_data


def parse(file_path, image_props):
    """Build an image from the pdf file at file_path"""
    f = codecs.open(file_path, "rb", "ascii") # Fixme: ascii will fail for values > 127
    text = f.read()

    page_sizes = _find_page_sizes(text)
    if page_sizes == 0:
        raise ifaint.LoadError("No pages found.")

    stream_list = _find_streams(text)
    if len(stream_list) == 0:
        raise ifaint.LoadError("No data streams found.")
    if len(stream_list) != len(page_sizes):
        raise ifaint.LoadError("Data stream count does not match page count.")

    meta_data_list = _find_faint_meta(text)
    if len(meta_data_list) != 0 and len(meta_data_list) != len(page_sizes):
        raise ifaint.LoadError("Faint meta data count does not match page count.")
    if len(meta_data_list) == 0:
        meta_data_list = [{} for num in page_sizes]

    for stream, size, meta_data in zip(stream_list, page_sizes, meta_data_list):
        frame_props = image_props.add_frame(size)
        _parse_stream(stream, frame_props, size, meta_data)
