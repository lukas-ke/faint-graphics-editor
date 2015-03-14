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

import ifaint
from faint.pdf.util import escape_string

from math import cos, sin, atan2, pi
__all__ = ("object_to_stream",)


def _arrow_line_end(tip_x, tip_y, angle, linewidth):
    return (tip_x + cos(angle) * 15 * (linewidth / 2.0),
            tip_y + sin(angle) * 15 * (linewidth / 2.0))

def _arrowhead(stream, lineWidth, x0, y0, x1, y1):
    angle = _rad_angle(x0, y0, x1, y1)
    orth = angle + pi / 2.0
    ax0 = cos(orth) * 10 * (lineWidth / 3.0)
    ay0 = sin(orth) * 10 * (lineWidth / 3.0)

    ax1 = cos(angle) * 15 * (lineWidth / 2.0)
    ay1 = sin(angle) * 15 * (lineWidth / 2.0)

    leX, leY = _arrow_line_end(x0, y0, angle, lineWidth)
    row = len(stream.stream.split("\n"))
    arrowheadPos = len(stream.stream)
    stream.triangle(x0 + ax0 + ax1, y0 + ay0 + ay1,
                     x0, y0,
                     x0 - ax0 + ax1, y0 - ay0 + ay1)
    stream.line(x1, y1, leX, leY)
    return _create_metadata(row, "faint-arrow", (x0, y0))


def _create_metadata(line, name, values):
    return "%d:%s" % (line, name) + "".join([" " + str(v) for v in values])


def _rad_angle(x0, y0, x1, y1):
    x1b = x1 - x0
    y1b = y1 - y0
    return atan2(y1b, x1b)


def _set_dash(stream, object):
    if object.linestyle == 'long_dash':
        w = object.linewidth * 2
        stream.dash(w, w)
    else:
        stream.dash_off()


def _add_path(stream, path_pts, doc_h, scale_x, scale_y):
    for pt in path_pts:
        if pt[0] == 'A':
            # FIXME
            pass

        elif pt[0] == 'C':
            x1, y1, x2, y2, x3, y3 = _transform_points(pt[1:], doc_h, scale_x, scale_y)
            stream.cubicspline(x2, y2, x3, y3, x1, y1)

        elif pt[0] == 'Z':
            stream.close()

        elif pt[0] == 'M':
            x, y = _transform_points(pt[1:], doc_h, scale_x, scale_y)
            stream.move(x, y)

        elif pt[0] == 'L':
            x, y = _transform_points(pt[1:], doc_h, scale_x, scale_y)
            stream.line_to(x,y)
        else:
            assert(False) # Fixme

def _stroke_and_or_fill(stream, object):
    fillStyle = object.fillstyle
    if fillStyle == 'border':
        stream.fgcol(*_to_pdf_color(object.fg))
        stream.stroke()
    elif fillStyle == 'fill':
        stream.bgcol(*_to_pdf_color(object.fg))
        stream.fill()
    elif fillStyle == 'border+fill':
        stream.fgcol(*_to_pdf_color(object.fg))
        stream.bgcol(*_to_pdf_color(object.bg))
        stream.stroke_and_fill()


def _to_pdf_color(faint_color):
    """Converts a Faint color tuple (range 0-255, rgb[a]) to a
    pdf color (range 0.0-1.0, rgb)
    """

    cls = faint_color.__class__

    if cls in (ifaint.LinearGradient, ifaint.RadialGradient):
        # Fixme: Add support for gradients
        faint_color = faint_color.get_stop(0)[1]
    elif cls is ifaint.Pattern:
        # Fixme: Add support for patterns
        return 0.0,0.0,0.0

    # Fixme: What of alpha?
    return (faint_color[0] / 255.0,
            faint_color[1] / 255.0,
            faint_color[2] / 255.0)


def _transform_points(faint_points, doc_h, scale_x, scale_y):
    pdf_points = []
    for i in range(len(faint_points)):
        if i % 2 == 0:
            pdf_points.append(faint_points[i] * scale_x)
        else:
            pdf_points.append(doc_h - faint_points[i] * scale_y)
    return pdf_points


def object_to_stream(s, object, doc_h, scale_x, scale_y):
    """Adds objects to the stream (s).
    Returns extra meta-data (for pdf comments) as a list."""

    meta = []
    if object.get_type() == 'Line':
        _set_dash(s, object)
        s.fgcol(*_to_pdf_color(object.fg))
        s.linewidth(object.linewidth)
        points = _transform_points(object.get_points(), doc_h, scale_x, scale_y)
        if len(points) == 4:
            x1, y1, x0, y0 = points
            if object.arrow == 'front':
                s.bgcol(*_to_pdf_color(object.fg))
                arrowPos = _arrowhead(s, object.linewidth, x0, y0, x1, y1)
                meta.append(arrowPos)
            else:
                s.line(x1, y1, x0, y0)
        else:
            s.polyline(points)

    elif object.get_type() == 'Rectangle':
        t = object.tri
        p0 = t.p0()
        p1 = t.p1()
        p2 = t.p2()
        p3 = t.p3()
        _set_dash(s, object)
        s.linewidth(object.linewidth)
        points = _transform_points(
            (p0[0], p0[1],
             p1[0], p1[1],
             p3[0], p3[1],
             p2[0], p2[1]),
            doc_h, scale_x, scale_y)

        s.polygon(points)
        _stroke_and_or_fill(s, object)

    elif object.get_type() == 'Text Region':
        s.bgcol(*_to_pdf_color(object.fg))
        x, y = object.tri.p0()
        textHeight = object.get_text_height()
        lineSpacing = object.get_text_height()
        y += textHeight
        x *= scale_x
        y *= scale_y
        y = doc_h - y
        for num, item in enumerate(object.get_text_lines()):
            textStr = escape_string(item[1])
            s.text(x, y - num * (textHeight + lineSpacing), object.fontsize,
                   textStr)

    elif object.get_type() == 'Spline':
        s.fgcol(*_to_pdf_color(object.fg))
        s.linewidth(object.linewidth)
        _set_dash(s, object)
        points = _transform_points(object.get_points(), doc_h, scale_x, scale_y)
        s.spline(points)

    elif object.get_type() == 'Polygon':
        _set_dash(s, object)
        s.linewidth(object.linewidth)
        points = _transform_points(object.get_points(), doc_h, scale_x, scale_y)
        s.polygon(points)
        _stroke_and_or_fill(s, object)

    elif object.get_type() == 'Path':
        _set_dash(s, object)
        s.linewidth(object.linewidth)
        _add_path(s, ifaint.get_path_points(object), doc_h, scale_x, scale_y)
        _stroke_and_or_fill(s, object)

    elif object.get_type() == 'Group':
        for i in range(object.num_objs()):
            object_to_stream(s, object.get_obj(i), doc_h, scale_x, scale_y)

    elif object.get_type() == 'Ellipse':
        _set_dash(s, object)
        s.linewidth(object.linewidth)
        row = len(s.stream.split("\n"))

        t = object.tri
        x, y = t.p0()
        w = t.width()
        h = t.height()
        meta.append(_create_metadata(row, "faint-ellipse", (x, y, w, h)))

        x *= scale_x
        y *= scale_y
        w *= scale_x
        h *= scale_y
        y = doc_h - y
        y -= h
        s.ellipse(x, y, w, h)
        _stroke_and_or_fill(s, object)

    return meta
