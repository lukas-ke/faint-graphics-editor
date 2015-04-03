#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2015 Lukas Kemmer
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
from . import util
from . parse.parse_util import mul_matrix_pt


def fill_style(stroke_str, fill_str, state):
    """Parses SVG stroke and fill attributes, returns a Faint Settings
    object.

    """
    settings = ifaint.Settings()
    if stroke_str == "none" and fill_str != "none":
        settings.fillstyle = 'f'
        settings.fg = parse_color(fill_str, "1.0", state)
    elif stroke_str != "none" and fill_str == "none":
        settings.fillstyle = 'b'
        settings.fg = parse_color(stroke_str, "1.0", state)
    else:
        settings.fillstyle = 'bf'
        settings.fg = parse_color(stroke_str, "1.0", state)
        settings.bg = parse_color(fill_str, "1.0", state)
    return settings


def mul_matrix_tri(m, tri): # pylint:disable=invalid-name
    """Multiples the matrix with the tri. Returns a new tri."""
    return ifaint.Tri(mul_matrix_pt(m, tri.p0()),
                      mul_matrix_pt(m, tri.p1()),
                      mul_matrix_pt(m, tri.p2()))


def to_faint_cap_str(svg_cap):
    """Parses an svg line cap string and returns the equivalent Faint
    line cap string.

    """
    # Fixme: Review, from old
    if svg_cap == 'butt':
        return 'flat'
    elif svg_cap == 'round':
        return 'round'
    else:
        return 'flat'



def remove_fill(settings):
    """Removes fill from the settings object, preserving stroke (i.e.
    border in Faint).

    """
    if settings.fillstyle == 'fill':
        settings.fillstyle = 'none'

    elif settings.fillstyle == 'fill+border':
        settings.fillstyle = 'border'
        return


def remove_stroke(settings):
    """Removes the stroke (i.e. border in Faint) from the settings object,
    preserving fill.

    """
    if settings.fillstyle == 'border':
        settings.fillstyle = 'none'
    elif settings.fillstyle == 'fill+border':
        settings.fg = settings.bg
        settings.fillstyle = 'fill'
