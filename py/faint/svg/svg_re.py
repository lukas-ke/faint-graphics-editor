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

"""Regexps for SVG1.1, 4: Types"""

# Number allows more concise syntax in SVG attributes than style
# properties (SVG 1.1, 4 Types)

number_attr = (
    # Real with optional exponent
    r"(?:[+-]?[0-9]*(?:[.][0-9]+(?:[Ee]\d+)?))|"
    # Integer with optional exponent
    r"(?:[+-]?\d+(?:[Ee]\d+)?)")

percentage = '(' + number_attr + ')' + '(?=%)'

length_unit_attr = "(?i)(?:em|ex|px|in|cm|mm|pt|pc|[%])?" # Fixme: Not case ins.

# <length>
# Regex provides two groups: (value, unit)
length_attr = "(%s)(%s)" % (number_attr, length_unit_attr)

coordinate = length_attr

transform_list = r'\w+\(.*?\)'

url_reference = r'url\(#(.*?)\)'
