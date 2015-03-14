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

"""Builds Faint images from for SVG read from string or file."""

__all__ = ["parse_doc", "parse_svg_string"]

from math import atan2
import re
import faint.svg.wrapped_etree as ET
import faint.svg.svg_re as svg_re
from faint.svg.parse.parse_util import (
    svg_error,
    Matrix,
    clean_path_definition,
    parse_color,
    parse_color_noref,
    parse_color_stop_offset,
    dict_union,
    center_based_to_rect,
    maybe_id_ref,
    extract_local_xlink_href,
    match_children,
    parse_embedded_image_data,
    ABSOLUTE_UNIT_FACTORS
)

from faint.svg.util import (
    arrow_line_end,
    deg2rad,
    rad_angle,
)

from . parse import (
    ParseState,
    get_style_dict,
    svg_length_attr,
    svg_length_attr_dumb,
)

from . ifaint_util import (mul_matrix_tri,
                           to_faint_cap_str,
                           fill_style)
import ifaint

_DEBUG_SVG = False

def parse_color_stops(nodes, state):
    """Parses the color stop nodes for a gradient node.
    Returns a list of offsets to colors.

    """
    stop_list = []
    for stop in [st for st in nodes if st.tag == ns_svg('stop')]:
        color_str, opacity_str = parse_color_stop_style(stop.get('style', ''))
        color_str = stop.get('stop-color', color_str)
        opacity_str = stop.get('stop-opacity', opacity_str)
        offset = parse_color_stop_offset(stop.get("offset"))

        # Fixme: Parse opacity str instead of just floating it
        faint_color = parse_color_noref(color_str, float(opacity_str), state)
        stop_list.append((offset, faint_color))
    return stop_list


def set_name(node, object_id, props):
    name = node.get('id')
    if name is not None:
        props.set_obj_name(object_id, name)


# Fixme: Rename id_to_node, it maps to parsed objects
def parse_defs(node, state, id_to_etree_node=None):
    """Recursively parses an SVG defs-node."""
    if id_to_etree_node is None:
        id_to_etree_node = {}
    for child in node:
        ref_id = child.get('id')
        if ref_id is not None:
            id_to_etree_node[ref_id] = child

    for child, func in match_children(node, svg_defs_content_functions):
        item = func(child, state, id_to_etree_node)

        if item is None or item.__class__ is int:
            # I guess "None" is failed parsing? Could
            # throw instead.
            # Fixme: int is probably an object id.
            # Should be able to reference those as well.
            state.add_warning("Ignored referenced item in <defs>")

        elif item == 'marker':
            # Ignore ignored markers for now, since I add them for
            # arrowhead, arrowtail.
            pass
        elif item == 'calibration':
            # Fully handled by function.
            pass
        else:
            ref_id, obj = item
            state.add_ref_item(ref_id, obj)


class SvgLiteralError(ifaint.LoadError):
    """Exception raised for invalidly expressed SVG literals."""
    def __init__(self, literal_type, literal):
        super().__init__("Invalid %s: %s" % (literal_type, literal))
        self.literal = literal


class SvgLengthError(SvgLiteralError):
    """Exception raised for invalidly expressed SVG lengths."""
    def __init__(self, literal):
        super().__init__("length", literal)


class SvgCoordError(SvgLiteralError):
    """Exception raised for invalidly expressed SVG coordinates."""
    def __init__(self, literal):
        super().__init__("coordinate", literal)

def parse_points(points_str):
    """Parses the points specification for polyline and polygon
    elements (SVG 1.1, 9.7.1).

    """

    # Treat #-# as # -#
    points_str = points_str.replace('-', ' -')
    return [float(s) for s in re.split("[\x20\x09\x0D\x0A]+|[,]",
      points_str) if s != ""]


def parse_pattern_node(node, state, id_to_etree_node):
    """Parses a pattern element"""

    state = state.updated(node)

    node_id = node.get('id')
    if node_id is None:
        # Ignore patterns without identifiers
        return None

    for child in node:
        if child.tag == ns_svg('image'):
            encoding, data = parse_embedded_image_data(child.get(ns_xlink('href')))
            if encoding == "png":
                return node_id, ifaint.Pattern(ifaint.bitmap_from_png_string(data))
    state.add_warning('Failed parsing pattern with id=%s' % node_id)
    return None


def get_linked_stops(xlink, state, id_to_etree_node):
    """Returns a referenced list of color-stops for a gradient.

    Retrieves the referenced id from the xlink, and retrieves its
    stops, if already available as a faint Gradient object in the state,
    otherwise looks up the target node in id_to_etree node, and tries to
    parse it as a gradient.

    """

    ref_id = extract_local_xlink_href(xlink)
    linked = state.ids.get(ref_id, None)

    if linked is None:
        if ref_id in id_to_etree_node:
            other = id_to_etree_node[ref_id]
            if other.tag == ns_svg('linearGradient'):
                linked_id, linked = parse_linear_gradient_node(other, state, id_to_etree_node)
            elif other.tag == ns_svg('radialGradient'):
                linked_id, linked = parse_radial_gradient_node(other, state, id_to_etree_node)
        if linked is None:
            state.add_warning("Referenced gradient not found: %s " % ref_id)

    if linked is not None:
        stops = []
        for num in range(linked.get_num_stops()):
            stops.append(linked.get_stop(num))
        return stops
    return []


def parse_faint_calibration(node, state, id_to_etree_node=None):
    points = tuple([svg_coord_attr(node.get(s,'0'), state)
              for s in ('x1','y1','x2','y2')])

    length = svg_length_attr(node.get('length'), state)
    unit = node.get('unit')
    state.props.set_calibration((points, length, unit))
    return 'calibration'


def parse_linear_gradient_node(node, state, id_to_etree_node=None):
    """Parses a linear gradient element"""

    state = state.updated(node)

    node_id = node.get('id')
    if node_id is None:
        # Ignore gradients without identifiers
        return None

    x1 = svg_coord_attr(node.get('x1', '0.0'), state)
    x2 = svg_coord_attr(node.get('x2', '0.0'), state)
    y1 = svg_coord_attr(node.get('y1', '0.0'), state)
    y2 = svg_coord_attr(node.get('y2', '0.0'), state)

    dx = x2 - x1
    dy = y2 - y1
    angle = atan2(dy, dx)

    stops = []
    stops = parse_color_stops(list(node), state)

    if len(stops) == 0:
        # No color stops, follow references
        # FIXME: Can there both be stops and a reference?
        xlink = node.get(ns_xlink('href'))
        if xlink is not None:
            stops = get_linked_stops(xlink, state, id_to_etree_node)
        else:
            # Ignore gradients without stops
            # (Note: Need to add handling of referenced gradients)
            state.add_warning("linearGradient with id=%s has no color-stops" % node_id)
            return None
    if len(stops) == 0:
        state.add_warning("linearGradient with id=%s has no color-stops" % node_id)

        return None

    return node_id, ifaint.LinearGradient(angle, *stops)


def parse_radial_gradient_node(node, state, id_to_etree_node=None):
    """Parses a radial gradient node."""
    # Fixme: Review, from old
    node_id = node.get('id')
    if node_id is None:
        # Ignore gradients without identifiers
        return None

    state = state.updated(node)

    stops = []
    stops = parse_color_stops(list(node), state)

    if len(stops) == 0:
        # No color stops, follow references
        # FIXME: Can there both be stops and a reference?
        xlink = node.get(ns_xlink('href'))
        if xlink is not None:
            stops = get_linked_stops(xlink, state, id_to_etree_node)
        else:
            # Ignore gradients without stops
            state.add_warning("radialGradient with id=%s has no color-stops" % node_id)
            return None
    if len(stops) == 0:
        state.add_warning("radialGradient with id=%s has no color-stops" % node_id)
        return None

    return node_id, ifaint.RadialGradient(stops)


def parse_color_stop_style(style):
    """Return the color and opacity from the style as strings.
    Returns None for color-string and 1.0 for opacity if not present

    """

    style_dict = get_style_dict(style)
    stop_opacity = style_dict.get('stop-opacity', '1.0')
    color_str = style_dict.get('stop-color', None)
    return color_str, stop_opacity


def parse_faint_tri_attr(node):
    """Parses a faint:tri attribute from the node"""
    tri_str = node.get(ns_faint("tri"))
    p0, p1, p2 = pairs(parse_points(tri_str))
    return ifaint.Tri(p0, p1, p2)


def parse_ellipse(node, state):
    """Parses an SVG <ellipse>-element."""
    state = state.updated(node)
    bounding_rect = center_based_to_rect(
        svg_coord_attr(node.get("cx", "0.0"), state),
        svg_coord_attr(node.get("cy", "0.0"), state),
        svg_coord_attr(node.get("rx", "0.0"), state),
        svg_coord_attr(node.get("ry", "0.0"), state))

    return state.props.Ellipse(bounding_rect, state.settings)


def parse_group(node, state, id_to_etree_node=None):
    """Parses an SVG <g>-element."""
    # Fixme: Use id_to_etree_node
    state = state.updated(node)
    # Parse the children of the group
    object_ids = []
    for child, func in match_children(node, svg_group_content_functions):
        result = func(child, state)
        if result.__class__ is not int:
            # Expected an object id from the function
            state.add_warning("Failed parsing child %s of group %s" %
                              (maybe_id_ref(child), maybe_id_ref(node)))
        else:
            object_ids.append(result)

    if len(object_ids) == 0:
        return

    return state.props.Group(object_ids)


def parse_path(node, state):
    """Parses an SVG <path>-element"""

    state = state.updated(node)
    path_def = node.get('d')
    if path_def is None:
        state.add_warning("Ignored path-element without definition "
                          "attribute%s." % maybe_id_ref(node))
        return

    path_def = clean_path_definition(path_def)
    if len(path_def) == 0:
        state.add_warning("Ignored path-element without definition "
                          "attribute%s." % maybe_id_ref(node))
        return

    try:
        path_id = state.props.Path(path_def, state.settings)
    except ValueError:
        ifaint.copy_text(node.get('d')) # Fixme: Remove
        state.add_warning("Failed parsing a path definition%s." %
                          maybe_id_ref(node))
        return

    return path_id


def parse_path_as_ellipse(node, state):
    """Parses an ellipse with a faint:tri attribute to an ellipse.

    Faint saves ellipses as paths for some reason (probably to
    skew/transform only the points, not the stroked edge).

    """
    state = state.updated(node)
    tri = parse_faint_tri_attr(node)
    # Fixme: Accept Tri in props.Ellipse
    ellipse_id = state.props.Ellipse((0, 0, 1, 1), state.settings)
    state.props.set_obj_tri(ellipse_id, tri)
    return ellipse_id


def parse_polygon(node, state):
    """Parses an SVG <path>-element."""
    state = state.updated(node)
    points = parse_points(node.get('points', ''))
    if len(points) % 2 != 0:
        state.add_warning("Odd number of coordinates for polygon%s." %
                          maybe_id_ref(node))
        # SVG 1.1 F2 "Error processing" says
        # Render polyline and polygons with invalid points up to the
        # erroneous point.
        points = points[:-1]

    return state.props.Polygon(points, state.settings)


def parse_polyline(node, state):
    """Parses an SVG <polyline>-element."""
    state = state.updated(node)

    points = parse_points(node.get('points'))

    if state.settings.arrow == 'front':
        x0, y0, x1, y1 = points[-4:]
        angle = rad_angle(x0,y0,x1,y1)
        x, y = arrow_line_end(x1, y1, angle, state.settings.linewidth)
        points[-2] = x
        points[-1] = y


    if len(points) % 2 != 0:
        state.add_warning("Odd number of coordinates for polyline%s." %
                          maybe_id_ref(node))
        # SVG 1.1 F2 "Error processing" says
        # Render polyline and polygons with invalid points up to the
        # erroneous point.
        points = points[:-1]

    return state.props.Line(points, state.settings)


def parse_faint_text_size_attrs(node, state):
    """Parses the faint-added width, height attributes"""

    # Get name-spaced or un-namespaced width, height
    # (added by Faint regardless)
    width_str = (node.get(ns_faint('width')) or
                  node.get('width'))
    height_str = (node.get(ns_faint('height')) or
                  node.get('height'))
    if width_str is not None:
        w = svg_length_attr(width_str, state)
    else:
        w = 200 # Fixme
    if height_str is not None:
        h = svg_length_attr(height_str, state)
    else:
        h = 200
    return not (width_str is None or height_str is None), w, h


def svg_baseline_to_faint(text_id, state):
    """Convert the position of the text object in props with text_id.

    SVG anchors at baseline, Faint at top of text.
    """
    dy = state.props.get_obj_text_height(text_id)
    tri = state.props.get_obj_tri(text_id)
    tri.translate(0, -dy)
    state.props.set_obj_tri(text_id, tri)


def parse_text(node, state):
    """Parses an SVG <text>-element."""
    state = state.updated(node)

    # Fixme: Should support <coordinate-list>
    x = svg_length_attr(node.get('x', '0'), state)
    y = svg_length_attr(node.get('y', '0'), state)

    boxed, w, h = parse_faint_text_size_attrs(node, state)
    # Fixme: Parse dx, dy lists etc.

    settings = ifaint.Settings()
    settings.update_all(state.settings)

    # Fixme: These assignments will overwrite inherited settings
    #        even if node doesn't have the retrieved setting
    default_bound = '1' if boxed else '0'
    settings.bounded = node.get(ns_faint('bounded'), default_bound) == '1'

    settings.valign = node.get(ns_faint('valign'), 'top')

    # TODO: If no unit specified size is processed as a height in the user
    #       coordinate system.
    #       Otherwise converts the length into user coordinate system

    fontsize_str = node.get('font-size')
    if fontsize_str is not None:
        # Fixme: Handle 'inherit' generically somehow
        if fontsize_str != 'inherit':
            settings.fontsize = svg_length_attr(fontsize_str, state)

    style = get_style_dict(node.get('style', ''))
    if 'fill' in style:
        settings.fg = parse_color(style['fill'], "1.0", state)

    anchor = node.get('text-anchor', 'start')
    if anchor == 'start':
        settings.halign = 'left'
    elif anchor == 'middle':
        settings.halign = 'center'
        x -= w / 2 # fixme
    elif anchor == 'end':
        settings.halign = 'right'

    text_str = None
    settings.parsing = node.get(ns_faint("parsing")) == '1'
    if settings.parsing:
        raw_node = node.find(ns_faint('raw'))
        if raw_node is not None:
            text_str = raw_node.text

    if text_str is None:
        text_str = ""
        if node.text:
            text_str += node.text

        for child in node:
            if child.tag == ns_svg('tspan'):
                # Fixme: Parse settings from the tspans
                if len(child.text) > 0:
                    text_str += child.text


                if child.get(ns_faint('hardbreak'), '0') == '1':
                    text_str += "\n"

    text_id = state.props.Text((x, y, w, h), text_str, settings)
    svg_baseline_to_faint(text_id, state)    
    state.transform_object(text_id)
    return text_id

def ns_svg(item_name):
    """Prepends the svg xml-namespace to the item name."""
    return '{http://www.w3.org/2000/svg}' + item_name


def ns_xlink(item_name):
    """Prepends the xlink xml-namespace to the item name."""
    return '{http://www.w3.org/1999/xlink}' + item_name


def ns_faint(item_name):
    """Prepends the faint xml-namespace to the item name."""
    return '{http://www.code.google.com/p/faint-graphics-editor}' + item_name


def is_faint_type(node, expected):
    """True if the node has a faint:type attribute with value matching
    expected. Allows mapping SVG shapes etc. to specific Faint object
    types.

    """
    return node.get(ns_faint('type')) == expected


def pairs(iterable):
    """Return a list which pairs up the neighbouring items in tuples.
    Will lose the last item if uneven item count.
    """
    i = iter(iterable)
    return list(zip(i, i))


def parse_circle(node, state):
    """Parses an SVG <circle>-element."""
    state = state.updated(node)
    cx = svg_coord_attr(node.get('cx', '0'), state)
    cy = svg_coord_attr(node.get('cy', '0'), state)
    r = svg_length_attr(node.get('r', '0'), state)
    rect = center_based_to_rect(cx, cy, r, r)
    return state.props.Ellipse(rect, state.settings)


def parse_polygon_as_rect(node, state):
    """Skewed Faint-rectangles are saved as polygons (marked with
    faint:type="rect").

    """
    state = state.updated(node)

    name = node.get('id')

    # Fixme: Pass Tri to props.rect instead of defaulting like this
    rect_id = state.props.Rect((0, 0, 1, 1), state.settings, name)

    p0, p1, p3, p2 = pairs(parse_points(node.get('points')))
    del p3
    state.props.set_obj_tri(rect_id, ifaint.Tri(p0, p1, p2))
    return rect_id


def parse_image_as_background(node, state):
    """Parses the embedded content of image node with a faint:background
    attribute and sets it as the background.

    """
    image_str = node.get(ns_xlink('href'))
    image_type, data = parse_embedded_image_data(image_str)
    assert image_type == 'png'
    state.props.set_background(ifaint.bitmap_from_png_string(data))


def parse_image(node, state):
    """Parses an SVG <image>-element."""
    assert node.get(ns_faint('background')) is None
    state = state.updated(node)

    x = svg_length_attr(node.get('x', '0.0'), state)
    y = svg_length_attr(node.get('y', '0.0'), state)
    w = svg_length_attr(node.get('width', '0.0'), state)
    h = svg_length_attr(node.get('height', '0.0'), state)

    image_str = node.get(ns_xlink('href'))
    if image_str is None:
        state.add_warning("Ignored image element with no data.")
        return

    image_type, image_data = parse_embedded_image_data(image_str)
    bitmap_func = IMAGE_TYPE_TO_BITMAP.get(image_type)
    if bitmap_func is None:
        state.add_warning('Ignored image element with unsupported type: %s' %
            image_type)
        return

    bmp = bitmap_func(image_data)
    return state.props.Raster((x, y, w, h), bmp, state.settings)


def parse_image_custom(node, state):
    """Parses an SVG <image> element into a faint raster background if
    marked-up as such, otherwise as a Faint raster object.

    """
    if node.get(ns_faint('background')) is not None:
        return parse_image_as_background(node, state)
    else:
        return object_common(parse_image)(node, state)


def parse_line(node, state):
    """Parses an SVG <line> element."""
    state = state.updated(node)

    points = [svg_coord_attr(node.get(s, '0'), state)
           for s in ('x1', 'y1', 'x2', 'y2')]

    if state.settings.arrow == 'front':
        x0, y0, x1, y1 = points
        angle = rad_angle(x0,y0,x1,y1)
        x, y = arrow_line_end(x1, y1, angle, state.settings.linewidth)
        points[-2] = x
        points[-1] = y

    return state.props.Line(points, state.settings)


def parse_path_custom(node, state):
    """Parses an SVG <path> element as a Faint Ellipse if marked-up as
    such, otherwise as a Faint Path.

    """
    return (parse_path_as_ellipse(node, state) if is_faint_type(node, 'ellipse')
            else parse_path(node, state))


def parse_polygon_custom(node, state):
    """Parses an SVG <polygon> element as a Faint Rectangle if marked-up
    as such, otherwise as a Faint Polygon.

    """
    return (parse_polygon_as_rect(node, state) if is_faint_type(node, 'rect')
            else parse_polygon(node, state))


def parse_rect_as_background(node, state):
    """Parses a marked-up rect-node to a background color.

    """
    color = parse_color(node.get('fill'), '1.0', state)
    state.props.set_background((color, state.props.get_size()))


def parse_rect(node, state):
    """Parses an SVG <rect> element."""
    state = state.updated(node)
    pts = [svg_coord_attr(node.get(s, '0'), state)
           for s in ('x', 'y', 'width', 'height')]
    rx = svg_length_attr(node.get('rx', '0'), state)
    ry = svg_length_attr(node.get('ry', '0'), state)
    state.settings.rx = rx
    state.settings.ry = ry
    r = state.props.Rect(pts, state.settings)
    state.transform_object(r)
    return r

def parse_rect_custom(node, state):
    """Parses an SVG <rect> element as the Canvas background color if
    marked-up as such, otherwise as a regular Rectangle.

    """
    if node.get(ns_faint('background')) is not None:
        return parse_rect_as_background(node, state)
    else:
        return object_common(parse_rect)(node, state)


def parse_switch(node, state):
    """Parses a switch node"""
    for child in node:
        if child.tag not in svg_switch_element_content_functions:
            continue

        requiredExtensions = child.get('requiredExtensions')
        if requiredExtensions is not None and requiredExtensions != "":
            # Faint supports no extensions.
            continue

        requiredFeatures = child.get('requiredFeatures')
        if requiredFeatures is not None:
            rfs = requiredFeatures.split(" ")
            unsupported = [rf.strip() for rf in rfs if
                           rf not in supported_svg_features]
            if len(unsupported) != 0:
                # At least one required feature was unsupported.
                continue

        systemLanguage = child.get('systemLanguage', state.system_language)
        if systemLanguage == state.system_language:
            return svg_switch_element_content_functions[child.tag](child, state)


    state.add_warning("No supported child node of SVG switch-element")


def _not_implemented(node, state, *args): #pylint:disable=unused-argument
    if _DEBUG_SVG:
        state.add_warning("Support not implemented: %s" % node.tag)

def _not_implemented_marker(node, state, *args): #pylint:disable=unused-argument
    if _DEBUG_SVG:
        state.add_warning("Support not implemented: %s" % node.tag)
    return 'marker'

# Dictionary of functions for converting embedded image-element data
# to a Bitmap. The data must be decoded from e.g. base64 first.
IMAGE_TYPE_TO_BITMAP = {
    "jpeg": ifaint.bitmap_from_jpg_string,
    "png": ifaint.bitmap_from_png_string,
}

# pylint:disable=invalid-name

svg_animation_element_functions = {
    ns_svg('animate') : _not_implemented,
    ns_svg('animateColor') : _not_implemented,
    ns_svg('animateMotion') : _not_implemented,
    ns_svg('animateTransform'): _not_implemented,
    ns_svg('set'): _not_implemented
}

svg_descriptive_element_functions = {
    ns_svg('desc'): _not_implemented,
    ns_svg('metadata'): _not_implemented,
    ns_svg('title'): _not_implemented
}

svg_gradient_element_functions = {
    # SVG 1.1
    ns_svg('linearGradient') : parse_linear_gradient_node,
    ns_svg('radialGradient') : parse_radial_gradient_node
}


def object_common(func):
    """Wraps an object-parse function, and adds common functionality,
    instead of scattering it in the individual parse functions.

    """

    def update_object(node, state):
        obj_id = func(node, state)
        if obj_id is not None:
            state.transform_object(obj_id)
            set_name(node, obj_id, state.props)
        else:
            print(node)
        return obj_id

    return update_object


svg_shape_element_functions = {
    ns_svg('circle'): object_common(parse_circle),
    ns_svg('ellipse'): object_common(parse_ellipse),
    ns_svg('line'): object_common(parse_line),
    ns_svg('path'): object_common(parse_path_custom),
    ns_svg('polygon'): object_common(parse_polygon_custom),
    ns_svg('polyline'): object_common(parse_polyline),
    ns_svg('rect'): parse_rect_custom,
}

svg_structural_element_functions = {
    ns_svg('defs'): parse_defs,
    ns_svg('g'): object_common(parse_group),
    ns_svg('svg'): _not_implemented, # nested SVG
    ns_svg('symbol'): _not_implemented,
    ns_svg('use'): _not_implemented
}

svg_content_functions = dict_union(
    svg_animation_element_functions,
    svg_descriptive_element_functions,
    svg_shape_element_functions,
    svg_structural_element_functions,
    svg_gradient_element_functions,
    {
        ns_svg('a'): _not_implemented,
        ns_svg('altGlyphDef'): _not_implemented,
        ns_svg('clipPath'): _not_implemented,
        ns_svg('color-profile'): _not_implemented,
        ns_svg('cursor'): _not_implemented,
        ns_svg('filter'): _not_implemented,
        ns_svg('font'): _not_implemented,
        ns_svg('font-face'): _not_implemented,
        ns_svg('foreignObject'): _not_implemented,
        ns_svg('image') : parse_image_custom,
        ns_svg('marker'): _not_implemented_marker,
        ns_svg('mask'): _not_implemented,
        ns_svg('pattern'): _not_implemented,
        ns_svg('script'): _not_implemented,
        ns_svg('style'): _not_implemented,
        ns_svg('switch'): parse_switch,
        ns_svg('text') : object_common(parse_text),
        ns_svg('view') : _not_implemented
    }
)

svg_group_content_functions = dict_union(
    svg_animation_element_functions,
    svg_descriptive_element_functions,
    svg_shape_element_functions,
    svg_structural_element_functions,
    svg_gradient_element_functions,
    {
        ns_svg('a'): _not_implemented,
        ns_svg('altGlyphDef'): _not_implemented,
        ns_svg('clipPath') : _not_implemented,
        ns_svg('color-profile') : _not_implemented,
        ns_svg('cursor'): _not_implemented,
        ns_svg('filter'): _not_implemented,
        ns_svg('font'): _not_implemented,
        ns_svg('font-face'): _not_implemented,
        ns_svg('foreignObject'): _not_implemented,

        # Fixme: Shouldn't image in group go directly for the
        # non-background parsing?
        ns_svg('image') : parse_image_custom,

        ns_svg('marker'): _not_implemented_marker,
        ns_svg('mask'): _not_implemented,
        ns_svg('pattern'): _not_implemented,
        ns_svg('script'): _not_implemented,
        ns_svg('style'): _not_implemented,
        ns_svg('switch'): parse_switch,
        ns_svg('text') : parse_text,
        ns_svg('view'): _not_implemented,
    })

svg_switch_element_content_functions = dict_union(
    svg_animation_element_functions,
    svg_descriptive_element_functions,
    svg_shape_element_functions,
    {
        ns_svg('a'): _not_implemented,
        ns_svg('foreignObject'): _not_implemented,
        ns_svg('g'): parse_group,
        ns_svg('image') : parse_image,
        ns_svg('svg'): _not_implemented, # Nested SVG
        ns_svg('switch'): parse_switch,
        ns_svg('text'): parse_text,
        ns_svg('use'): _not_implemented,
    })

svg_descriptive_element_functions = {
    ns_svg('desc') : _not_implemented,
    ns_svg('metadata') : _not_implemented,
    ns_svg('title') : _not_implemented,
}

svg_defs_content_functions = dict_union(
    svg_animation_element_functions,
    svg_descriptive_element_functions,
    # todo: shape
    # todo: structural
    svg_structural_element_functions,
    svg_gradient_element_functions,
    {
      ns_svg('a'): _not_implemented,
      ns_svg('altGlyphDef'): _not_implemented,
      ns_svg('clipPath'): _not_implemented,
      ns_svg('colorProfile'): _not_implemented,
      ns_svg('cursor'): _not_implemented,
      ns_svg('filter'): _not_implemented,
      ns_svg('font'): _not_implemented,
      ns_svg('font-face'): _not_implemented,
      ns_svg('foreignObject'): _not_implemented,
      ns_svg('image'): _not_implemented,
      ns_svg('marker'): _not_implemented_marker,
      ns_svg('mask'): _not_implemented,
      ns_svg('pattern'): parse_pattern_node,
      ns_svg('script'): _not_implemented,
      ns_svg('style'): _not_implemented,
      ns_svg('switch'): parse_switch,
      ns_svg('text'): _not_implemented,
      ns_svg('view'): _not_implemented,
      ns_faint('calibration'): parse_faint_calibration,
    })

supported_svg_features = [

    # Not sure, group
    "http://www.w3.org/TR/SVG11/feature#SVG",

    # Not sure
    "http://www.w3.org/TR/SVG11/feature#SVGDOM",

    # Not totally.
    "http://www.w3.org/TR/SVG11/feature#SVG-static",

    # Probably untrue
    "http://www.w3.org/TR/SVG11/feature#SVGDOM-static",

    # No animation support at all
    #"http://www.w3.org/TR/SVG11/feature#SVG-animation",
    #"http://www.w3.org/TR/SVG11/feature#SVGDOM-animation",
    #"http://www.w3.org/TR/SVG11/feature#SVG-dynamic",
    #"http://www.w3.org/TR/SVG11/feature#SVGDOM-dynamic",

    # Support for attributes:
    # id, xml:base, xml:space attributes
    #"http://www.w3.org/TR/SVG11/feature#CoreAttribute",

    # Support for elements:
    # svg, g, defs, desc, title, metadata, symbol, use
    "http://www.w3.org/TR/SVG11/feature#Structure",

    # Support for elements:
    # svg, g, defs, desc, title, metadata, use
    "http://www.w3.org/TR/SVG11/feature#BasicStructure",

    # Support for enable-background property
    #"http://www.w3.org/TR/SVG11/feature#ContainerAttribute",

    # Support for the switch element, and the attributes:
    # requiredFeatures, requiredExtensions, systemLanguage
    "http://www.w3.org/TR/SVG11/feature#ConditionalProcessing",

    # Support for the image element
    "http://www.w3.org/TR/SVG11/feature#Image",

    # Support for the style element
    # (Not true)
    "http://www.w3.org/TR/SVG11/feature#Style",

    # Support for clip and overflow properties of viewport
    #"http://www.w3.org/TR/SVG11/feature#ViewportAttribute",

    # Support for the shape elements:
    # rect, circle, line, polyline, polygon, ellipse, path.
    "http://www.w3.org/TR/SVG11/feature#Shape",

    # Support for text, tspan, tref, textPath, altGlyph, altGlyphDef,
    # altGlyphItem, glyphRef.
    "http://www.w3.org/TR/SVG11/feature#Text",

    # Support for the text element.
    "http://www.w3.org/TR/SVG11/feature#BasicText",

    # Support for the properties:
    # color, fill, fill-rule, stroke, stroke-dasharray,
    # stroke-dashoffset, stroke-linecap, stroke-linejoin,
    # stroke-miterlimit, stroke-width, color-interpolation,
    # color-rendering.
    "http://www.w3.org/TR/SVG11/feature#PaintAttribute",

    # Support for the properties: color, fill, fill-rule, stroke,
    # stroke-dasharray, stroke-dashoffset, stroke-linecap,
    # stroke-linejoin, stroke-miterlimit, stroke-width,
    # color-rendering.
    "http://www.w3.org/TR/SVG11/feature#BasicPaintAttribute",

    # Support for opacity, stroke-opacity, fill-opacity
    # Fixme: 'opacity'? Check that out.
    "http://www.w3.org/TR/SVG11/feature#OpacityAttribute",

    # Support for the properties display, image-rendering,
    # pointer-events, shape-rendering, text-rendering visibility
    "http://www.w3.org/TR/SVG11/feature#GraphicsAttribute",

    # Support for display and visibility properties
    "http://www.w3.org/TR/SVG11/feature#BasicGraphicsAttribute",

    # Support for the marker element
    #"http://www.w3.org/TR/SVG11/feature#Marker",

    # Support for the color-profile element
    #"http://www.w3.org/TR/SVG11/feature#ColorProfile",

    # Support for elements linearGradient, radialGradient and stop
    "http://www.w3.org/TR/SVG11/feature#Gradient",

    # Support for the pattern element
    #"http://www.w3.org/TR/SVG11/feature#Pattern",

    # Support for the clipPath element and clip-path, clip-rule
    # properties
    #"http://www.w3.org/TR/SVG11/feature#Clip",

    # Support for the clipPath element and clip-path property
    #"http://www.w3.org/TR/SVG11/feature#BasicClip",

    # Support for the mask element
    #"http://www.w3.org/TR/SVG11/feature#Mask",

    # Support for the elements filter, feBlend, feColorMatrix,
    # feComponentTransfer, feComposite, feConvolveMatrix,
    # feDiffuseLighting, feDisplacementMap, feFlood, feGaussianBlur,
    # feImage, feMerge, feMergeNode, feMorphology, feOffset,
    # feSpecularLighting, feTile, feDistantLight, fePointLight,
    # feSpotLight, feFuncR, feFuncG, feFuncB, feFuncA.
    #"http://www.w3.org/TR/SVG11/feature#Filter",

    # Support for the elements filter, feBlend, feColorMatrix,
    # feComponentTransfer, feComposite, feFlood, feGaussianBlur,
    # feImage, feMerge, feMergeNode, feOffset, feTile, feFuncR,
    # feFuncG, feFuncB, feFuncA.
    #"http://www.w3.org/TR/SVG11/feature#BasicFilter",

    # Support for attributes onunload, onabort, onerror, onresize,
    # onscroll and onzoom.
    #"http://www.w3.org/TR/SVG11/feature#DocumentEventsAttribute",

    # Support for the attributes onfocusin, onfocusout, onactivate, onclick,
    # onmousedown, onmouseup, onmouseover, onmousemove, onmouseout,
    # onload.
    #"http://www.w3.org/TR/SVG11/feature#GraphicalEventsAttribute",

    # Support for the attributes onbegin, onend, onrepeat, onload
    #"http://www.w3.org/TR/SVG11/feature#AnimationEventsAttribute",

    # Support for the cursor element
    #"http://www.w3.org/TR/SVG11/feature#Cursor",

    # Support for the a element.
    #"http://www.w3.org/TR/SVG11/feature#Hyperlinking",

    # Support for the xlink:type, xlink:href, xlink:role,
    # xlink:arcrole, xlink:title, xlink:show and xlink:actuate
    # elements.
    # (Fixme: Only xlink:href in rare cases)
    "http://www.w3.org/TR/SVG11/feature#XlinkAttribute",

    # Support for the externalResourcesRequired attribute
    #"http://www.w3.org/TR/SVG11/feature#ExternalResourcesRequired",

    # Support for the view element.
    #"http://www.w3.org/TR/SVG11/feature#View",

    # Support for the script element.
    #"http://www.w3.org/TR/SVG11/feature#Script",

    # Support for the animate, set, animateMotion, animateTransform,
    # animateColor and mpath elements.
    #"http://www.w3.org/TR/SVG11/feature#Animation",

    # Support for the font, font-face, glyph, missing-glyph, hkern,
    # vkern, font-face-src, font-face-uri, font-face-format,
    # font-face-name elements.
    # (Fixme: Very limited support)
    "http://www.w3.org/TR/SVG11/feature#Font",

    # Support for the elements font, font-face, glyph, missing-glyph,
    # hkern, font-face-src, font-face-name.
    "http://www.w3.org/TR/SVG11/feature#BasicFont",

    # The foreignObject element
    #"http://www.w3.org/TR/SVG11/feature#Extensibility",
]

# pylint:enable=invalid-name

def svg_coord_attr(coord_str, state):
    """Parses an svg coordinate attribute."""
    # Fixme: Duplicates length_attr
    m = re.match(svg_re.coordinate, coord_str)
    if m is None:
        raise SvgCoordError(coord_str)
    value, unit = m.groups()
    if unit == '%':
        # Fixme: How should this work?
        if float(value) == 0:
            return 0
        else:
            return (state.containerSize[0] * 100) / float(value)
    elif unit in ABSOLUTE_UNIT_FACTORS:
        return float(value) * ABSOLUTE_UNIT_FACTORS[unit]
    elif unit in ('em', 'ex'):
        state.add_warning("Unsupported unit: %s" % unit)
        return float(value)
    else:
        state.add_warning("Invalid unit: %s" % unit)
        return float(value)


def check_svg_size(size):
    """Raises exception if invalid size for SVG node."""
    if size[0] <= 0:
        raise ifaint.LoadError("SVG element has negative width")
    elif size[1] <= 0:
        raise ifaint.LoadError("SVG element has negative height")
    return size


def svg_size(svg_node, image_props, view_box_str):
    """Returns w, h from the svg node."""
    # Fixme: Truncating the svg width, height (Faint limitation)
    # Fixme: Use what for container size here? Easier if not outermost, I guess

    default_w = view_box_str[2]
    default_h = view_box_str[3]

    svg_w = svg_node.get('width', default_w)
    svg_h = svg_node.get('height', default_h)

    # Fixme: I guess viewbox should be converted, not float:ed
    w2 = int(svg_length_attr_dumb(svg_w, image_props, full_span=float(default_w)))
    h2 = int(svg_length_attr_dumb(svg_h, image_props, full_span=float(default_h)))
    return check_svg_size((w2, h2))


def get_viewbox(node, default=('0','0','640','480')):
    """Returns the viewbox from the node as a sequence of four strings,
    x, y, w, h."""

    # Available for: svg, symbol (via use), image, foreignObject
    viewBox = node.get('viewBox') # pylint:disable=invalid-name
    if viewBox is None:
        return default

    # Fixme: Handle preserveAspectRatio
    x, y, w, h = [v for v in viewBox.split(' ')]
    return x, y, w, h


def parse_svg_root_node(svg_node, image_props, system_language):
    """Parses the SVG-root node and recurses into child nodes. Creates the
    frame from the SVG root node properties.

    """

    # Fixme: Check if SVG element can have transform

    view_box = get_viewbox(svg_node)
    props = image_props.add_frame(svg_size(svg_node, image_props, view_box))
    state = ParseState(props, system_language=system_language)

    # Fixme: Convert instead of float
    x0, y0 = float(view_box[0]), float(view_box[1])
    if x0 != 0 or y0 != 0:
        state.ctm = Matrix.translation(-x0, -y0) * state.ctm

    for child in svg_node:
        if child.tag in svg_content_functions:
            # Fixme: I guess there could be a transform already?
            svg_content_functions[child.tag](child, state)


def parse_doc(path, image_props, system_language="en"):
    """Parses the SVG document at the given file path, using the
    image_props to build an image.

    """

    try:
        ET.register_namespace("svg", "{http://www.w3.org/2000/svg}")
        tree = ET.parse(path)
        root = tree.getroot()
        if root.tag == ns_svg("svg"):
            parse_svg_root_node(root, image_props, system_language)
        else:
            raise ifaint.LoadError("Root element was not <svg>.")
    except svg_error as e:
        raise ifaint.LoadError(str(e))

def parse_svg_string(xml_string, image_props, system_language="en"):
    """Parses the SVG document in the given string, using the image_props
    to build an image.

    """

    try:
        ET.register_namespace("svg", "{http://www.w3.org/2000/svg}")
        root = ET.fromstring(xml_string)
        if root.tag == ns_svg("svg"):
            parse_svg_root_node(root, image_props, system_language)
        else:
            raise ifaint.LoadError("Root element was not <svg>.")
    except svg_error as e:
        raise ifaint.LoadError(str(e))
