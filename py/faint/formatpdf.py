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

"""PDF load and save functions for ifaint.add_format."""

import faint.pdf.parse_pdf as parse_pdf
import faint.pdf.write_pdf as write_pdf

def load(filename, imageprops):
    """Build an image from the PDF file specified by filename, using
    the image props"""
    return parse_pdf.parse(filename, imageprops)

def save(filename, canvas):
    """Save the image to the specified PDF file"""
    return write_pdf.write( filename, canvas )
