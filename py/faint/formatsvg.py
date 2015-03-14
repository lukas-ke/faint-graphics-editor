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

"""SVG load and save functions for ifaint.add_format."""

import faint.svg.write_svg as write_svg
import faint.svg.parse_svg as parse_svg

def load(filename, imageprops):
    """Build an image from the SVG file specified by filename, using the
    imageprops.

    """
    return parse_svg.parse_doc(filename, imageprops, "en")

def save(filename, canvas):
    """Save the image to the specified SVG file."""
    return write_svg.write(filename, canvas)
