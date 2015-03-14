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
import re

# Section-title regexes
title1 = re.compile("^= (.*) =$")
title2 = re.compile("^== (.*) ==$")
title3 = re.compile("^=== (.*) ===$")

# Comment to end of line
re_comment = re.compile(r'#.*')

# Summary of a page
re_summary = re.compile('^summary\:"(.+)"$')

# Instruction to summarize child pages
re_summarize_children = re.compile('^child_summary$')

# Label for cross-referencing
label = re.compile("^label\:(.*?)$")

# Bullet points for bullet lists
bullet = re.compile("^\* (.*)$")

# Image filename from images/
re_image = re.compile(r"\\image\((.*?)\)")

# Image map description file
re_image_map = re.compile(r"\\imagemap\((.*?)\)")

# Image filename from graphics/
re_graphic = re.compile(r"\\graphic\((.*?)\)")

# Include raw content
re_include = re.compile(r"\\include\((.*?)\)")

# Include raw content
re_verbatim_include = re.compile(r"\\verbatim_include\((.*?)\)")

# Include other page
re_insert = re.compile(r"\\insert\((.*?)\)")

# Font styles
bold = re.compile("\*(.*?)\*")
italic = re.compile("\'(.*?)\'")

# Centered text
center = re.compile("/(.*?)/")

# Horizontal line
hr = re.compile("^---$")

# Reference to a label or page
re_ref = re.compile(r"\\ref\((.*?)\)")

# External reference
re_extref = re.compile(r"\\extref\((.*?)\)")

# Include Python code file
re_py_sample = re.compile(r"\\py_example\((.*)\)")

# A table row (with one or more cells)
table_row = re.compile("^\|\|.*\|\|$")

# Table style definition
table_style = re.compile("^tablestyle:(.*)$")

# Table widths definition
table_widths = re.compile("^tablewidths:(.*)$")

re_withbind = re.compile("withbind\:(.*?);")
re_bind = re.compile("bind\:(.*?);")
re_bindlist = re.compile(r"^\\bindlist\(\)$")
re_configpath = re.compile(r"\\pyconfigpath")
