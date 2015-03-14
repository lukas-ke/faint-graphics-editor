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

"""Load and save functions for zipped svg files."""

import gzip
import faint.svg.parse_svg as parse_svg
import faint.svg.write_svg as write_svg

def load(filename, imageprops):
    """Load image from the zipped svg file."""
    with gzip.open(filename, 'rb') as f:
        file_content = f.read()
        return parse_svg.parse_svg_string(file_content, imageprops, "en")

def save(filename, canvas):
    """Save the image to the specified file as zipped svg."""
    data = write_svg.to_string(canvas).encode('utf-8')
    with gzip.open(filename, 'wb') as f:
        f.write(data)
