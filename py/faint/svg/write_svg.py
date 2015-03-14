#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2014 Lukas Kemmer
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

"""Functions for creating SVG images from Faint images.

"""

__all__ = ["write", "to_string"]

import base64
from faint.svg.util import arrow_line_end, rad2deg, rad_angle
from faint.svg.write import SvgBuildState
from math import atan2, cos, sin, tan
import xml.etree.ElementTree as ET

from faint.formatutil import open_for_writing_binary
import ifaint

_PREAMBLE = """<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN"
  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
"""


def set_name_as_id(obj, element):
    """Sets the Faint-object name (if any) as id for the element."""
    name = obj.name
    if name is not None:
        element.set('id', name) # Fixme: Use namespace <- Fixme: Why, what? Aha, not NCName?


def to_svg_cap(cap):
    """Turns a Faint line cap into an SVG line cap"""
    return FAINT_TO_SVG_CAP[cap]


def to_rgb_color(color):
    """Turns a Faint rgb-color tuple into an svg string."""
    return "rgb%s" % str(color[0:3])


def to_svg_color(color, state):
    """Turns a Faint color tuple, pattern or gradient into an svg color
    description or defs-reference.

    """
    if state.should_link(color):
        return 'url(#%s)' % state.get_defs_id(color)
    return to_rgb_color(color)


def to_opacity_str(color):
    """Returns the alpha component, if available, converted to an SVG
    opacity. Defaults to '1.0'."""

    if color.__class__ == ifaint.LinearGradient:
        return "1.0" # Fixme: Hard coded opacity for gradient
    elif color.__class__ == ifaint.RadialGradient:
        return "1.0"
    elif color.__class__ == ifaint.Pattern:
        return "1.0" # Fixme: Hard coded opacity for pattern
    elif len(color) == 3:
        return "1.0"
    return "%s" % str(color[3] / 255.0)


def svg_fill_style(obj, state):
    """Returns an SVG style string with style settings for the passed in
    Faint object.

    """

    fill = obj.fillstyle
    stroke_width = obj.linewidth
    fg = obj.fg
    bg = obj.bg

    if fill == 'border':
        return to_style({
            'fill': 'none',
            'stroke-width' : str(stroke_width),
            'stroke' : to_svg_color(fg, state),
            'stroke-opacity' : to_opacity_str(fg)})

    elif fill == "fill":
        return to_style({
            'stroke': 'none',
            'fill': to_svg_color(fg, state),
            'fill-opacity': to_opacity_str(fg)})

    elif fill == "border+fill":
        return to_style({
            "stroke": to_svg_color(fg, state),
            "stroke-width": str(stroke_width),
            "fill": to_svg_color(bg, state),
            "fill-opacity": to_opacity_str(bg),
            "stroke-opacity": to_opacity_str(fg)})
    elif fill == "none":
        return to_style({
            'stroke': 'none',
            'fill': 'none'})
    else:
        assert(False)



def svg_line_dash_style(obj):
    """Returns an SVG style string describing the dash for the passed in
    Faint object.

    """
    if obj.linestyle == 'long_dash':
        w = obj.linewidth
        # Dash and space length is twice the width

        # Fixme: Check Cairo stuff, avoid duplication, this will
        # probably be a setting.
        return "stroke-dasharray:%d,%d;" % (w * 2, w * 2)

    return ""


def svg_line_join_style(obj):
    """Returns an SVG style string describing the linejoin for the passed
    in Faint object.

    """

    join = obj.get_settings().join
    return "stroke-linejoin:%s;" % join


def svg_line_style(obj, state):
    """Returns an SVG style string describing the stroke color, width,
    opacity and cap for the passed in Faint object.

    """

    color = obj.fg
    return to_style({
        'stroke': to_svg_color(color, state),
        'stroke-width': str(obj.linewidth),
        'stroke-opacity': to_opacity_str(color),
        'stroke-linecap': to_svg_cap(obj.get_settings().cap)})


def svg_no_fill():
    """Returns a transparent SVG fill style"""
    return "fill:none;"


def to_style(style_dict):
    """Creates an svg style string from a dict."""
    return ";".join(['%s:%s' % (k, style_dict[k])
        for k in sorted(style_dict.keys())]) + ";"


def tri_attr(tri):
    """Returns a tuple with the attribute name for a Faint tri, and the
    values for the passed in tri as a string.

    Should be unpacked when adding to an element:
    - e.set(*tri_attr(tri))

    """

    # Fixme: Add tri-list-converter
    x0, y0 = tri.p0()
    x1, y1 = tri.p1()
    x2, y2 = tri.p2()
    return ('faint:tri',
        "%f,%f %f,%f %f,%f" % (x0, y0, x1, y1, x2, y2))


def get_transform_and_offset(obj):
    """Return the value for the SVG transform attribute and an x-offset for
    the elements x-position depending on the transform

    """
    # Fixme: translate instead of returning offset.
    tri = obj.tri
    angle_rad = tri.angle
    ori_x, ori_y = tri.p0()
    x_offset = 0
    tf = ""

    if angle_rad != 0:
        tf += 'rotate(%f,%f,%f)' % (rad2deg(angle_rad), ori_x, ori_y)
        # Unrotate for the rest of the values
        tri.angle = 0.0

    # Fixme: This was commented in old write_svg.
    # Fixme: This is incorrect.
    skew = tri.skew
    if skew != 0:
        if len(tf) > 0:
            tf = tf + " "
        skew_angle_rad = atan2(skew, tri.p0()[1] - tri.p2()[1])
        tf = tf + 'skewX(%f)' % rad2deg(skew_angle_rad)
        x_offset = - tri.p0()[1] * tan(skew_angle_rad)
    return tf, x_offset


def encode_bitmap_base64_png(bmp):
    binary_png = ifaint.encode_bitmap_png(bmp)
    return ("data:image/png;base64," +
            base64.b64encode(binary_png).decode('ascii'))


def create_background(image):
    """Creates a background color or image."""
    return (create_background_color(image)
            if ifaint.one_color_bg(image.get_frame())
            else create_background_image(image))


def create_background_image(image):
    """Creates an image-element marked up as the Faint raster layer.
    Contains the background as an embedded base64-encoded png.

    """
    base64_png = encode_bitmap_base64_png(image.get_stamped_bitmap())
    w, h = image.get_size()

    element = ET.Element('image')
    element.set('faint:background', '1')
    element.set('x', '0')
    element.set('y', '0')
    element.set('width', str(w))
    element.set('height', str(h))
    element.set('xlink:href', base64_png)
    return element

def create_background_color(image):
    """Creates a full-sized, filled rect-element marked up as the Faint
    background color.

    """
    color = image.get_pixel(0,0)
    element = ET.Element('rect')
    element.set('faint:background', '1')
    element.set('x', '0')
    element.set('y', '0')
    element.set('width', '100%')
    element.set('height', '100%')
    element.set('fill', to_rgb_color(color))
    return element

def create_calibration(calibration):
    """Creates a calibration line for the defs"""
    x1,y1,x2,y2 = calibration[0]
    length = calibration[1]
    unit = calibration[2]

    element = ET.Element('faint:calibration')
    element.set('x1', str(x1))
    element.set('y1', str(y1))
    element.set('x2', str(x2))
    element.set('y2', str(y2))

    element.set('length', str(length))
    element.set('unit', str(unit))
    return element

def create_defs_child_element(obj, element_id):
    """Creates an SVG element to be appended to the <defs>-element."""
    return DEFS_CREATORS[obj.__class__](obj, element_id)


def create_element(obj, state):
    """Create an ElementTree Element from a Faint object"""
    element = CREATORS[obj.get_type()](obj, state)
    set_name_as_id(obj, element)
    return element


def create_ellipse_path(obj, state):
    """Creates a path-element from a Faint ellipse. Marks it up as a
    Faint-ellipse to allow reloading."""

    # Fixme: Save regular ellipses as ellipses
    element = ET.Element('path')
    element.set('d', ifaint.to_svg_path(obj))

    style = svg_fill_style(obj, state) + svg_line_dash_style(obj)
    element.set('style', style)
    element.set(*tri_attr(obj.tri))
    element.set('faint:type', 'ellipse')
    return element


def create_group(obj, state):
    """Creates an SVG <g>-element from a Faint group."""

    group = ET.Element('g')
    for i in range(obj.num_objs()):
        # Fixme: object list retrieval
        child = obj.get_obj(i)
        group.append(create_element(child, state))

    return group


def create_image(obj, state): #pylint:disable=unused-argument
    """Creates an SVG <image>-element from a Faint Raster object."""

    element = ET.Element('image')

    tf, x_offset = get_transform_and_offset(obj)
    t = obj.tri
    x, y = t.p0()
    x += x_offset
    w = t.width()
    h = t.height()

    if len(tf) > 0:
        element.set('transform', tf)

    for k, v in zip(('x', 'y', 'width', 'height'), (x, y, w, h)):
        element.set(k, str(v))

    element.set('faint:bg-style', obj.bgstyle)
    element.set('faint:mask-color', 'rgb' + str(obj.bg))

    base64_png = encode_bitmap_base64_png(obj.get_bitmap())
    element.set('xlink:href', base64_png)

    return element


def create_line(obj, state):
    """Creates a <line> or <polyline> element."""
    if len(obj.get_points()) == 4:
        return create_simple_line(obj, state)
    return create_polyline(obj, state)


def create_color_stops(stops):
    """Creates a list of <stop>-elements for a linearGradient or
    radialGradient from a list of Faint color stops.

    """
    # Sorting the stops by offset is required in SVG to get the same
    # appearance as a Cairo gradient
    stops = sorted(stops, key=lambda x: x[0])

    elements = []
    for stop in stops:
        offset, color = stop
        el = ET.Element('stop')
        el.set('offset', '%d%%' % int(offset * 100))
        el.set('style', to_style({
            'stop-color': to_rgb_color(color),
            'stop-opacity': '1'}))
        elements.append(el)

    return elements

def create_linear_gradient(gradient, id_attribute):
    """Creates a linearGradient element from a Faint LinearGradient."""

    element = ET.Element('linearGradient')
    element.set('id', id_attribute)

    angle = gradient.get_angle()
    if angle == 0:
        x1, x2 = 0, 1
        y1, y2 = 0, 0
    else:
        x1 = 0
        x2 = cos(angle)
        if x2 < 0:
            x1, x2 = -x2, x1

        y1 = 0
        y2 = sin(angle)
        if y2 < 0:
            y1, y2 = -y2, y1
    element.set('x1', str(x1))
    element.set('y1', str(y1))
    element.set('x2', str(x2))
    element.set('y2', str(y2))

    element.extend(create_color_stops(gradient.get_stops()))
    return element

def create_path(obj, state):
    """Creates an SVG <path> element from a Faint Path."""

    path_str = ifaint.to_svg_path(obj)

    if len(path_str) == 0:
        # Fixme: Need a way to warn from save?
        print("Warning: Skipping empty path")
        return None

    element = ET.Element('path')
    element.set('d', path_str)
    style = (svg_fill_style(obj, state) +
             svg_line_dash_style(obj) +
             svg_line_join_style(obj))
    element.set('style', style)
    return element


def create_pattern(pattern, element_id):
    """Creates a <pattern> element from a Faint Pattern."""

    bmp = pattern.get_bitmap()
    w, h = bmp.get_size()

    element = ET.Element('pattern')

    element.set('id', element_id)
    element.set('x', '0')
    element.set('y', '0')
    element.set('width', str(w))
    element.set('height', str(h))

    if pattern.get_object_aligned():
        # Fixme: Add support for object-aligned patterns
        pass
    else:
        # Image aligned
        element.set('patternUnits', 'userSpaceOnUse')
        element.set('patternContentUnits', 'userSpaceOnUse')

    base64_png = encode_bitmap_base64_png(bmp)
    image = ET.Element('image')
    image.set('width', str(w)),
    image.set('height', str(h)),
    image.set('xlink:href', base64_png)
    element.append(image)
    return element


def create_polygon(obj, state):
    """Creates an SVG <polygon> element from a Faint Polygon."""

    element = ET.Element('polygon')
    points_str = ' '.join([str(coord) for coord in obj.get_points()])
    element.set('points', points_str)
    style = (svg_fill_style(obj, state) + svg_line_dash_style(obj) +
             svg_line_join_style(obj))
    element.set('style', style)
    return element


def create_polyline(obj, state):
    """Creates an SVG <polyline> element from a Faint Line."""

    element = ET.Element('polyline')
    points = obj.get_points()

    arrow = obj.arrow
    state.set_arrow(arrow)

    if arrow == 'front':
        angle = rad_angle(points[-2], points[-1], points[-4], points[-3])
        x, y = arrow_line_end(points[-2], points[-1],
            angle, obj.linewidth)
        points[-2] = x
        points[-1] = y

    point_str = ",".join([str(pt) for pt in points])
    element.set('points', point_str)

    style = svg_line_style(obj, state) + svg_line_dash_style(obj) + "fill:none"
    element.set('style', style)

    if arrow == 'front' or arrow == 'both':
        element.set('marker-end', 'url(#Arrowhead)')
    elif arrow == 'back' or arrow == 'both':
        element.set('marker-start', 'url(#Arrowtail)')
    return element


def create_radial_gradient(gradient, element_id):
    """Creates a radialGradient element from a Faint RadialGradient."""

    cx, cy = gradient.get_center()
    rx, ry = gradient.get_radii()

    element = ET.Element('radialGradient')
    element.set('id', element_id)
    element.set('cx', str(cx))
    element.set('cy', str(cy))
    element.set('rx', str(rx))
    element.set('ry', str(ry))

    element.extend(create_color_stops(gradient.get_stops()))
    return element


def create_rect_polygon(obj, state):
    """Creates an SVG polygon element from a Faint Rect."""

    element = ET.Element('polygon')
    point_str = str(obj.get_points())[1:-1]
    element.set('faint:type', 'rect')
    element.set('points', point_str)

    # Bundle as shape_style?
    style = (svg_fill_style(obj, state) +
             svg_line_dash_style(obj) +
             svg_line_join_style(obj))
    element.set('style', style)

    return element


def create_rounded_rect(obj, state):
    """Creates an SVG rect element from a Faint Rect,
    with roundedness-radius."""

    element = ET.Element('rect')
    t = obj.tri
    x, y = t.p0()
    element.set('x', str(x))
    element.set('y', str(y))
    element.set('width', str(t.width()))
    element.set('height', str(t.height()))
    element.set('rx', str(obj.rx))
    element.set('ry', str(obj.rx)) # Fixme: ry not supported yet.

    if (t.angle != 0):
        element.set('transform',
            'rotate(%f, %f, %f)' % (rad2deg(t.angle), x, y))

    # Bundle as shape_style?
    style = (svg_fill_style(obj, state) +
             svg_line_dash_style(obj) +
             svg_line_join_style(obj))
    element.set('style', style)

    return element


def create_rect(obj, state):
    if obj.rx == 0.0:
        return create_rect_polygon(obj, state)
    else:
        return create_rounded_rect(obj, state)

def create_simple_line(obj, state):
    """Creates an SVG <line> element from a Faint Line object."""

    element = ET.Element('line')
    points = obj.get_points()
    assert len(points) == 4
    element.set('x1', str(points[0]))
    element.set('y1', str(points[1]))

    arrow = obj.arrow
    state.set_arrow(arrow)

    # Fixme: Duplicates poly-line
    if arrow == 'front':
        angle = rad_angle(points[2], points[3], points[0], points[1])
        x, y = arrow_line_end(points[2], points[3], angle, obj.linewidth)
        element.set('x2', str(x))
        element.set('y2', str(y))
    else:
        element.set('x2', str(points[2]))
        element.set('y2', str(points[3]))

    element.set('style', svg_line_style(obj, state) + svg_line_dash_style(obj))

    if arrow == 'front' or arrow == 'both':
        element.set('marker-end', 'url(#Arrowhead)')
    if arrow == 'back' or arrow == 'both':
        element.set('marker-start', 'url(#Arrowtail)')

    return element


def create_spline_path(obj, state):
    """Creates an SVG path element from a Faint Spline object. Marks it up
    as a faint:spline to allow reloading.

    """

    element = ET.Element('path')
    element.set('d', ifaint.to_svg_path(obj))
    element.set('faint:type', 'spline')

    style = (svg_line_style(obj, state) +
             svg_line_dash_style(obj) +
             svg_no_fill())
    element.set('style', style)
    return element


def create_text(obj, state):
    """Creates an SVG <text> element from a Faint Text object."""

    element = ET.Element('text')

    # Axis-aligned tri
    tri = obj.tri
    tri.angle = 0.0

    x0, y0 = tri.p0()
    w = tri.width()
    h = tri.height()

    text_height = obj.get_text_height()
    baseline = obj.get_text_baseline()

    settings = obj.get_settings()

    bounded = '1' if settings.bounded else '0'
    element.set('faint:bounded', bounded)

    halign = settings.halign
    if halign == 'center':
        element.set('text-anchor', 'middle')
        x0 += w / 2
    elif halign == 'right':
        element.set('text-anchor', 'right')
        x0 += w

    valign = settings.valign
    if valign != 'top':
        element.set('faint:valign', valign)

    element.set('x', str(x0))
    element.set('y', str(y0 + baseline))
    element.set('width', str(w))
    element.set('height', str(h))

    element.set('style', to_style({
        'fill': to_svg_color(settings.fg, state),
        'font-size': str(settings.fontsize) + 'px',
        # Fixme: face vs family
        'font-family': settings.font,
        'font-style': 'italic' if settings.fontitalic else 'normal',
        'font-weight': 'bold' if settings.fontbold else 'normal'}))

    if settings.parsing:
        element.set('faint:parsing', '1')

    # Add the lines as tspan elements
    lines = obj.get_text_lines()
    line_tri = obj.tri
    line_tri.angle = 0.0

    if valign == 'middle':
        line_tri.offset_aligned(0, (tri.height() - text_height *
            len(lines)) / 2)
    elif valign == 'bottom':
        line_tri.offset_aligned(0, (tri.height() - text_height *
            len(lines)))

    for item in lines:
        hard_break = item[0] == 1
        line = item[1]
        width = item[2]
        tspan = ET.SubElement(element, 'tspan')
        lx, ly = line_tri.p0()
        if halign == 'center':
            lx += (tri.width() - width) / 2
        elif halign == 'right':
            lx += (tri.width() - width)

        tspan.set('x', str(lx))
        # Fixme: Use dy instead
        tspan.set('y', str(ly + baseline))

        if hard_break:
            # Fixme: I for some reason add extra whitespace to the end
            # when splitting. Presumably for caret placement
            tspan.text = line[:-1]
            tspan.set('faint:hardbreak', '1')
        else:
            tspan.text = line
        line_tri.offset_aligned(0, text_height)

        angle_deg = obj.angle_deg
        if angle_deg != 0:
            pivot_x, pivot_y = tri.p0()
            element.set('transform',
                        'rotate(%f,%f,%f)' % (angle_deg, pivot_x, pivot_y))

    if obj.parsing:
        raw_text = ET.SubElement(element, 'faint:raw')
        raw_text.text = obj.get_text_raw()

    return element


FAINT_TO_SVG_CAP = {
    'flat': 'butt',
    'round': 'round',
}

CREATORS = {
    "Ellipse" : create_ellipse_path,
    "Group" : create_group,
    "Line": create_line,
    "Path": create_path,
    "Polygon" : create_polygon,
    "Raster" : create_image,
    "Rectangle": create_rect,
    "Spline": create_spline_path,
    "Text Region" : create_text,
}


DEFS_CREATORS = {
    ifaint.LinearGradient: create_linear_gradient,
    ifaint.Pattern: create_pattern,
    ifaint.RadialGradient: create_radial_gradient,
}


def set_svg_size(svg_element, canvas):
    """Set the size of the <SVG>-element from the Canvas."""
    w, h = canvas.get_size()
    svg_element.set('width', str(w))
    svg_element.set('height', str(h))


def to_string(canvas):
    """Return an SVG-string for the Faint canvas"""

    ET.register_namespace("", "http://www.w3.org/2000/svg")
    root = ET.Element('svg')
    root.set('version', '1.1')
    root.set('xmlns', 'http://www.w3.org/2000/svg')
    root.set('xmlns:faint',
        'http://www.code.google.com/p/faint-graphics-editor')
    root.set('xmlns:xlink', "http://www.w3.org/1999/xlink")

    set_svg_size(root, canvas)

    state = SvgBuildState()
    defs = ET.SubElement(root, 'defs')
    root.append(create_background(canvas))

    # Build the tree
    for obj in canvas.get_objects():
        root.append(create_element(obj, state))

    # Add the defs created during building
    defs.extend([create_defs_child_element(obj, element_id)
        for element_id, obj in state.get_items()])

    defs.extend(state.get_marker_elements())

    calibration = canvas.get_calibration()
    if calibration is not None:
        defs.append(create_calibration(calibration))

    return _PREAMBLE + ET.tostring(root, encoding="unicode")


def write(path, canvas):
    """Writes the Faint canvas to the file specified by the path."""

    svg_str = to_string(canvas)
    with open_for_writing_binary(path) as svg_file:
        svg_file.write(svg_str.encode('utf-8'))
