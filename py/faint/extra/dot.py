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

"""GraphViz dot-file loading.

Call init_dot_format() to add support for loading .dot-files.

"""

import platform
import subprocess as sp

import faint
import faint.svg.parse_svg as parse_svg
from faint.extra.util import hide_console

__all__ = ["init_dot_format",
           "load_dot",
           "load_dot_string"]


def _run_dot(cmd, image_props, graph_input):
    stdin = Nonr if graph_input is None else sp.PIPE

    try:
        dot = sp.Popen(cmd,
                       stdout=sp.PIPE,
                       stderr=sp.PIPE,
                       stdin=stdin,
                       startupinfo=hide_console())
    except FileNotFoundError:
        image_props.set_error("GraphViz dot not available.")
        return

    out, err = dot.communicate(input=graph_input)

    if len(out) == 0:
        image_props.set_error("Nothing written to standard output by dot."
         "\n\n%s" % err.decode("ascii"))
    else:
        parse_svg.from_string(out, image_props)
        if len(err) > 0:
            image_props.add_warning("dot error output:\n\n" +
                                    err.decode("ascii"))


def load_dot(file_path, image_props):
    """Use dot to convert the file to SVG, and parse the SVG with the
    given ImageProps.

    """
    cmd = ["dot", "-Tsvg", file_path]
    _run_dot(cmd, image_props, graph_input=None)


def load_dot_string(txt, image_props):
    """Use dot to convert the graph description to SVG, and parse the SVG
    with the given ImageProps.

    """
    cmd = ["dot", "-Tsvg"]
    _run_dot(cmd, image_props, graph_input=txt)


def init_dot_format(app):
    """Adds a format for opening GraphViz dot-files. Requires that the
    dot-application is available on the path.

    """
    app.add_format(load_dot, None, "GraphViz dot", "dot")
