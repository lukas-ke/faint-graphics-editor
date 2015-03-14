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

"""GraphViz dot-file loading."""

import platform
import subprocess as sp

import ifaint
from faint.svg.parse_svg import parse_svg_string
from faint.extra.util import hide_console

__all__ = ["init_dot_format"]

def _load_dot(file_path, image_props):
    """Use dot to generate an svg, then load that SVG."""
    cmd = ["dot", "-Tsvg", file_path]
    try:
        dot = sp.Popen(cmd, stdout=sp.PIPE, stderr=sp.PIPE,
          startupinfo=hide_console())
    except FileNotFoundError:
        image_props.set_error("GraphViz dot not available.")
        return

    out, err = dot.communicate()
    if len(out) == 0:
        image_props.set_error("Nothing written to standard output by dot."
         "\n\n%s" % err.decode("ascii"))
    else:
        parse_svg_string(out, image_props)
        if len(err) > 0:
            image_props.add_warning("dot error output:\n\n" +
                                    err.decode("ascii"))

def init_dot_format():
    """Adds a format for opening GraphViz dot-files. Requires that the
    dot-application is available on the path.

    """
    ifaint.add_format(_load_dot, None, "GraphViz dot", "dot")
