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

"""Helper functions for parse_svg.

Note: To simplify testing, this file should not depend on ifaint.
Use the ifaint_util module instead for such utilities.

"""

from functools import reduce
from math import tan, cos, pi, sin, atan2
import re
import faint.svg.svg_re as svg_re

ABSOLUTE_UNIT_FACTORS = {
    '':  1.0,
    'px': 1.0,
    'pt': 1.25,
    'pc': 15, # 12 by Millenium Falcon
    'mm': 3.543307,
    'cm': 35.543307,
    'in': 90
}


X11_COLORS = {
    "aliceblue" : (240, 248, 255),
    "antiquewhite" : (250, 235, 215),
    "aqua" : (0, 255, 255),
    "aquamarine" : (127, 255, 212),
    "azure" : (240, 255, 255),
    "beige" : (245, 245, 220),
    "bisque" : (255, 228, 196),
    "black" : (0, 0, 0),
    "blanchedalmond" : (255, 235, 205),
    "blue" : (0, 0, 255),
    "blueviolet" : (138, 43, 226),
    "brown" : (165, 42, 42),
    "burlywood" : (222, 184, 135),
    "cadetblue" : (95, 158, 160),
    "chartreuse" : (127, 255, 0),
    "chocolate" : (210, 105, 30),
    "coral" : (255, 127, 80),
    "cornflowerblue" : (100, 149, 237),
    "cornsilk" : (255, 248, 220),
    "crimson" : (220, 20, 60),
    "cyan" : (0, 255, 255),
    "darkblue" : (0, 0, 139),
    "darkcyan" : (0, 139, 139),
    "darkgoldenrod" : (184, 134, 11),
    "darkgray" : (169, 169, 169),
    "darkgreen" : (0, 100, 0),
    "darkkhaki" : (189, 183, 107),
    "darkmagenta" : (139, 0, 139),
    "darkolivegreen" : (85, 107, 47),
    "darkorange" : (255, 140, 0),
    "darkorchid" : (153, 50, 204),
    "darkred" : (139, 0, 0),
    "darksalmon" : (233, 150, 122),
    "darkseagreen" : (143, 188, 143),
    "darkslateblue" : (72, 61, 139),
    "darkslategray" : (47, 79, 79),
    "darkturquoise" : (0, 206, 209),
    "darkviolet" : (148, 0, 211),
    "deeppink" : (255, 20, 147),
    "deepskyblue" : (0, 191, 255),
    "dimgray" : (105, 105, 105),
    "dodgerblue" : (30, 144, 255),
    "firebrick" : (178, 34, 34),
    "floralwhite" : (255, 250, 240),
    "forestgreen" : (34, 139, 34),
    "fuchsia" : (255, 0, 255),
    "gainsboro" : (220, 220, 220),
    "ghostwhite" : (248, 248, 255),
    "gold" : (255, 215, 0),
    "goldenrod" : (218, 165, 32),
    "gray" : (128, 128, 128),
    "green" : (0, 128, 0),
    "greenyellow" : (173, 255, 47),
    "honeydew" : (240, 255, 240),
    "hotpink" : (255, 105, 180),
    "indianred" : (205, 92, 92),
    "indigo" : (75, 0, 130),
    "ivory" : (255, 255, 240),
    "khaki" : (240, 230, 140),
    "lavender" : (230, 230, 250),
    "lavenderblush" : (255, 240, 245),
    "lawngreen" : (124, 252, 0),
    "lemonchiffon" : (255, 250, 205),
    "lightblue" : (173, 216, 230),
    "lightcoral" : (240, 128, 128),
    "lightcyan" : (224, 255, 255),
    "lightgoldenrodyellow" : (250, 250, 210),
    "lightgreen" : (144, 238, 144),
    "lightgrey" : (211, 211, 211),
    "lightpink" : (255, 182, 193),
    "lightsalmon" : (255, 160, 122),
    "lightseagreen" : (32, 178, 170),
    "lightskyblue" : (135, 206, 250),
    "lightslategray" : (119, 136, 153),
    "lightsteelblue" : (176, 196, 222),
    "lightyellow" : (255, 255, 224),
    "lime" : (0, 255, 0),
    "limegreen" : (50, 205, 50),
    "linen" : (250, 240, 230),
    "magenta" : (255, 0, 255),
    "maroon" : (128, 0, 0),
    "mediumaquamarine" : (102, 205, 170),
    "mediumblue" : (0, 0, 205),
    "mediumorchid" : (186, 85, 211),
    "mediumpurple" : (147, 112, 219),
    "mediumseagreen" : (60, 179, 113),
    "mediumslateblue" : (123, 104, 238),
    "mediumspringgreen" : (0, 250, 154),
    "mediumturquoise" : (72, 209, 204),
    "mediumvioletred" : (199, 21, 133),
    "midnightblue" : (25, 25, 112),
    "mintcream" : (245, 255, 250),
    "mistyrose" : (255, 228, 225),
    "moccasin" : (255, 228, 181),
    "navajowhite" : (255, 222, 173),
    "navy" : (0, 0, 128),
    "oldlace" : (253, 245, 230),
    "olive" : (128, 128, 0),
    "olivedrab" : (107, 142, 35),
    "orange" : (255, 165, 0),
    "orangered" : (255, 69, 0),
    "orchid" : (218, 112, 214),
    "palegoldenrod" : (238, 232, 170),
    "palegreen" : (152, 251, 152),
    "paleturquoise" : (175, 238, 238),
    "palevioletred" : (219, 112, 147),
    "papayawhip" : (255, 239, 213),
    "peachpuff" : (255, 218, 185),
    "peru" : (205, 133, 63),
    "pink" : (255, 192, 203),
    "plum" : (221, 160, 221),
    "powderblue" : (176, 224, 230),
    "purple" : (128, 0, 128),
    "red" : (255, 0, 0),
    "rosybrown" : (188, 143, 143),
    "royalblue" : (65, 105, 225),
    "saddlebrown" : (139, 69, 19),
    "salmon" : (250, 128, 114),
    "sandybrown" : (244, 164, 96),
    "seagreen" : (46, 139, 87),
    "seashell" : (255, 245, 238),
    "sienna" : (160, 82, 45),
    "silver" : (192, 192, 192),
    "skyblue" : (135, 206, 235),
    "slateblue" : (106, 90, 205),
    "slategray" : (112, 128, 144),
    "snow" : (255, 250, 250),
    "springgreen" : (0, 255, 127),
    "steelblue" : (70, 130, 180),
    "tan" : (210, 180, 140),
    "teal" : (0, 128, 128),
    "thistle" : (216, 191, 216),
    "tomato" : (255, 99, 71),
    "turquoise" : (64, 224, 208),
    "violet" : (238, 130, 238),
    "wheat" : (245, 222, 179),
    "white" : (255, 255, 255),
    "whitesmoke" : (245, 245, 245),
    "yellow" : (255, 255, 0),
    "yellowgreen" : (154, 205, 50),
}

SYSTEM_COLORS = {

    # These are apparently "Deprecated in CSS3". Unsure about the SVG status.
    # Added to pass SVG tests.
    # Modeled after the classic Hot dog stand theme.

    # Active window border
    'ActiveBorder': (0, 0, 0),

    # Active window caption
    'ActiveCaption': (0, 0, 0),

    # Background color of multiple document interface
    'AppWorkspace': (255, 255, 0),

    # Desktop background
    'Background': (255, 255, 0),

    # Face color for three-dimensional display elements.
    'ButtonFace': (198, 198, 198),

    # Highlight color for three-dimensional display elements (for
    # edges facing away from the light source).
    'ButtonHighlight': (255, 255, 255),

    # Shadow color for three-dimensional display elements.
    'ButtonShadow': (0, 0, 0),

    # Text on push buttons.
    'ButtonText': (0, 0, 0),

    # Text in caption, size box, and scrollbar arrow box.
    'CaptionText': (255, 255, 255),

    # Grayed (disabled) text. This color is set to #000 if the current
    # display driver does not support a solid gray color
    # ["ha-ha", Lukas anmärkning]
    'GrayText': (132, 132, 132),

    # Item(s) selected in a control.
    'Highlight': (0, 0, 0),

    # Text of item(s) selected in a control.
    'HighlightText': (0, 0, 0),

    # Inactive window border.
    'InactiveBorder':(255, 255, 0),

    # Inactive window caption.
    'InactiveCaption':(255, 255, 0),

    # Color of text in an inactive caption.
    'InactiveCaptionText':(255, 255, 255),

    # Background color for tooltip controls.
    'InfoBackground':(255, 255, 255), # Fixme: Verify against HDS

    # Text color for tooltip controls.
    'InfoText':(0, 0, 0),

    # Menu background.
    'Menu':(255, 255, 255),

    # Text in menus.
    'MenuText':(0, 0, 0),

    # Scroll bar gray area.
    'Scrollbar':(198, 198, 198),

    # Dark shadow for three-dimensional display elements.
    'ThreeDDarkShadow':(0, 0, 0),

    # Face color for three-dimensional display elements.
    'ThreeDFace':(255, 255, 255),

    # Highlight color for three-dimensional display elements.
    'ThreeDHighlight':(255, 255, 255),

    # Light color for three-dimensional display elements (for edges
    # facing the light source).
    'ThreeDLightShadow':(132, 132, 132),

    # Dark shadow for three-dimensional display elements.
    'ThreeDShadow':(0, 0, 0),

    # The color of the moon
    'ThreeWolfMoon':(163, 163, 128),

    # Window background
    'Window':(255, 0, 0),

    # Window frame.
    'WindowFrame':(0, 0, 0),

    # Text in windows.
    'WindowText':(0, 0, 0),
}


# Custom exception to keep this file free from ifaint-dependencies
# Should be transformed to ifaint.LoadError by parse_svg.
class svg_error(Exception):
    pass


def parse_color_stop_offset(svg_string):
    """Parses an offset for a gradient color stop"""

    match = re.match(svg_re.percentage, svg_string)
    if match:
        return float(match.group(0)) / 100

    match = re.match(svg_re.number_attr, svg_string)
    if match:
        # Ratio offset
        # Fixme: I think standard says clamp to 0.0-1.0
        return float(match.group(0))
    else:
        raise svg_error("Failed parsing gradient offset")


def match_children(node, node_function_dict):
    """Returns. a list of node, function pairs.

    Matches the children of the passed in node to parse functions in
    the passed in dictionary. The node tags are used as keys.

    Ignores nodes which get no match.

    """
    return [(ch, node_function_dict[ch.tag])
            for ch in node if ch.tag in node_function_dict]


def decode_base64(data):
    """Decodes a base64 string to binary."""
    import base64
    data = data.replace("\n", "")
    data = data.replace(" ", "")
    data = base64.b64decode(data)
    return data

BASE64_JPEG_PREFIX = "data:image/jpeg;base64,"
BASE64_PNG_PREFIX = "data:image/png;base64,"
def strip_mime(data):
    """Returns a pair of a type specifier and the data without the
    MIME-type, if its a mime type supported by this function. The type
    specifier will be either base64_jpg or base64_png.

    Returns None, "" on failure.

    """
    if data.startswith(BASE64_JPEG_PREFIX):
        return "base64_jpg", data[len(BASE64_JPEG_PREFIX):]
    elif data.startswith(BASE64_PNG_PREFIX):
        return "base64_png", data[len(BASE64_PNG_PREFIX):]
    else:
        return None, ""

class Matrix:
    """SVG transformation matrix (SVG 1.1, 7.4)
      a c e
      b d f
      0 0 1

    """
    def __init__(self, a=1, b=0, c=0, d=1, e=0, f=0):
        # pylint:disable=invalid-name
        self.a = a
        self.b = b
        self.c = c
        self.d = d
        self.e = e
        self.f = f

    @staticmethod
    def identity():
        """Creates an identity matrix"""
        return Matrix()

    @staticmethod
    def translation(dx, dy):
        """Creates a translation matrix"""
        return Matrix(1, 0, 0, 1, dx, dy)

    @staticmethod
    def scale(sx, sy):
        """Creates a scale matrix"""
        return Matrix(sx, 0, 0, sy, 0, 0)

    @staticmethod
    def skewX(rad): # pylint:disable=invalid-name
        """Creates a horizontal skew matrix"""
        return Matrix(1, 0, tan(rad), 1, 0, 0)

    @staticmethod
    def skewY(rad): # pylint:disable=invalid-name
        """Creates a vertical skew matrix"""
        return Matrix(1, tan(rad), 0, 1, 0, 0)

    @staticmethod
    def rotation(rad, pivot=None):
        """Creates a rotation matrix, optionally around pivot"""
        if pivot is None:
            return Matrix(cos(rad), sin(rad), -sin(rad), cos(rad), 0, 0)

        dx, dy = pivot # pylint:disable=unpacking-non-sequence
        return ((Matrix.translation(dx, dy) * Matrix.rotation(rad)) *
            Matrix.translation(-dx, -dy))

    def __mul__(self, m2):
        m1 = self
        return Matrix(
            a=m1.a * m2.a + m1.c * m2.b + m1.e * 0,
            b=m1.b * m2.a + m1.d * m2.b + m1.f * 0,
            c=m1.a * m2.c + m1.c * m2.d + m1.e * 0,
            d=m1.b * m2.c + m1.d * m2.d + m1.f * 0,
            e=m1.a * m2.e + m1.c * m2.f + m1.e * 1,
            f=m1.b * m2.e + m1.d * m2.f + m1.f * 1)

    def __str__(self):
        return "%s, %s, %s, %s, %s, %s" % (self.a, self.b, self.c,
                                           self.d, self.e, self.f)


def mul_matrix_pt(m, point): # pylint:disable=invalid-name
    """Multiplies the matrix with the point. Returns a new point."""
    x, y = point
    x2 = x * m.a + y * m.c + 1 * m.e
    y2 = x * m.b + y * m.d + 1 * m.f
    return x2, y2


def parse_embedded_image_data(image_string):
    """Returns a string describing the format, and a string with the raw
    image data.

    """
    encoding, data = strip_mime(image_string)
    if encoding == "base64_jpg":
        return "jpeg", decode_base64(data)
    elif encoding == "base64_png":
        return "png", decode_base64(data)
    return None, ""


def maybe_id_ref(node):
    """Return the node id as a string if available, prefixed by space,
    otherwise an empty string.

    """
    node_id = node.get('id')
    if node_id is not None:
        return " (id=%s)" % node_id
    return ""


def extract_url_reference(ref_string):
    """Extracts the id part from an in-document url reference"""
    match = re.match(svg_re.url_reference, ref_string)
    if match is None:
        return None
    return match.group(1)


def percent_to_rgb(r, g, b):
    """Transforms color values expressed as 0-100% into an rgb-tuple
    # Fixme: Review, from old
    of 0-255 integers"""
    r_fl = float(r[:-1])
    g_fl = float(g[:-1])
    b_fl = float(b[:-1])

    r = int(255 * r_fl / 100.0)
    g = int(255 * g_fl / 100.0)
    b = int(255 * b_fl / 100.0)
    return r, g, b


def parse_rgb_color(svg_color):
    """Parses an svg rgb color"""
    # Fixme: Review, from old

    assert svg_color.startswith("rgb")
    color = svg_color[3:]
    color = color.replace("(", "").replace(")", "")

    parts = color.split(",")
    if len(parts) == 4:
        # Fixme: Non-conformant terrible hack of doom
        r, g, b, a = [int(part) for part in parts]
        return r, g, b, a

    r, g, b = color.split(",")
    if r[-1] == '%' or g[-1] == '%' or b[-1] == '%':
        r, g, b = percent_to_rgb(r, g, b)
        return r, g, b, a
    else:
        return int(r), int(g), int(b), 255


def parse_color_noref(svg_color_str, opacity, state):
    """Parses an svg named-, hex- or rgb-color specification"""
    # Fixme: handle "currentColor"
    # Fixme: Review, from old

    a = int(255 * opacity)
    if svg_color_str == "currentColor":
        return state.currentColor

    elif svg_color_str == "inherit":
        # Fixme: Could probably be something else than currentColor
        return state.currentColor

    if svg_color_str[0:3] == "rgb":
        color = svg_color_str[3:]
        color = color.replace("(", "").replace(")", "")

        parts = color.split(",")
        if len(parts) == 4:
            # Fixme: Non-conformant terrible hack of doom
            r, g, b, a = [int(part) for part in parts]
            return r, g, b, a

        r, g, b = color.split(",")
        if r[-1] == '%' or g[-1] == '%' or b[-1] == '%':
            r, g, b = percent_to_rgb(r, g, b)
            return r, g, b, a
        else:
            return int(r), int(g), int(b), a
    elif svg_color_str[0] == "#":
        r, g, b = hex_to_rgb(svg_color_str)
        return r, g, b, a

    named = X11_COLORS.get(svg_color_str)
    if named is not None:
        r, g, b = named
        return r, g, b, a

    named = SYSTEM_COLORS.get(svg_color_str)
    if named is not None:
        r, g, b = named
        return r, g, b, a

    state.add_warning("Failed parsing color: %s" % svg_color_str)
    return 0, 0, 0, a


def parse_color(svg_color_str, opacity_str, state):
    """Parses an svg color attribute string."""
    # Fixme: Review, from old
    if svg_color_str.startswith("url"):
        ref_id = extract_url_reference(svg_color_str)
        referenced = state.ids.get(ref_id, None)
        if referenced is None:
            state.add_warning("Failed retrieving reference: %s" % svg_color_str)
            return 0, 0, 0, 255
        else:
            return referenced
    else:
        # Clamp out-of-range opacity)
        opacity = min(1.0, max(0.0, float(opacity_str)))
        return parse_color_noref(svg_color_str, opacity, state) # Fixme

def center_based_to_rect(cx, cy, rx, ry):
    """Returns x, y, w, h from the center and radius parameters."""
    return cx - rx, cy - ry, rx  * 2, ry * 2


def apply_transforms(transforms, matrix):
    """Applies all transform matrices in the list to the matrix. Returns
    the transformed matrix.

    """
    return matrix * reduce(Matrix.__mul__, transforms, Matrix.identity())


def extract_local_xlink_href(ref_string):
    """Extracts an id reference from xlink:href to an item in the same
    document from the attribute value"""
    # Fixme: Review, from old
    match = re.match(r"\#(.*)", ref_string)
    if match is None:
        return None
    return match.group(1)


def clean_path_definition(path_def):
    """Makes the path_def more managable"""
    # Fixme: Sucks
    return path_def.replace(",", " ")


def hex_to_rgb(hex_color):
    """Transforms a hexadecimal color string into an rgb-tuple of
    0-255 integers"""
    if len(hex_color) == 4:
        # Thank you w3c for this convenient shorthand form
        # (#fb0 is equal to #ffbb00)
        hex_color = "#" + hex_color[1]*2 + hex_color[2]*2 + hex_color[3]*2

    assert len(hex_color) == 7
    r = int(hex_color[1:3], 16)
    g = int(hex_color[3:5], 16)
    b = int(hex_color[5:7], 16)
    return r, g, b


def dict_union(*dicts):
    """Returns a union of the passed in dictionaries."""
    combined = dicts[0].copy()
    for d in dicts:
        combined.update(d)
    return combined
