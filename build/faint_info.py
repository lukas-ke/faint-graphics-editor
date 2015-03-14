#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2013 Lukas Kemmer
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

import os
import time
from os.path import join

FAINT_ROOT = (os.path.abspath("../") + "/").replace("\\","/")

HEADERS_TO_GENERATE = [

    # List of cpp-files to parse with gen_method_def.py for generating methoddef
    # include files
    #
    # The first item is the prefix for the methoddef name
    #   PyMethodDef <prefix>_methods[],
    # The second item is the cpp file to parse.
    # The methoddef-header will be named the same as the parsed cpp,
    # but with .cpp replaced by -methoddef.hh
    ('LinearGradient', '../python/py-linear-gradient.cpp'),
    ('Pattern', '../python/py-pattern.cpp'),
    ('RadialGradient', '../python/py-radial-gradient.cpp'),
    ('bitmap', '../python/py-bitmap.cpp'),
    ('canvas', '../python/py-canvas.cpp'),
    ('faint_interface', ('../python/py-functions.hh',
                         '../python/py-object-geo.cpp')),
    ('faintapp', '../python/py-app.cpp'),
    ('faintinterpreter', '../python/py-interpreter.cpp'),
    ('faintpalette', '../python/py-palette.cpp'),
    ('faintwindow', '../python/py-window.cpp'),
    ('frame', '../python/py-frame.cpp'),
    ('frame_props', '../python/py-frame-props.cpp'),
    ('grid', '../python/py-grid.cpp'),
    ('imageprops', '../python/py-image-props.cpp'),
    ('settings', '../python/py-settings.cpp'),
    ('smth', '../python/py-something.cpp'),
    ('tri', '../python/py-tri.cpp'),
    (None, '../python/py-common.hh'),
    (None, '../python/py-less-common.hh'),
]

GENERATED_HELP_PATH = '../help/source/generated'
GENERATED_METHOD_DEF_PATH = '../generated/python/method-def'


def get_src_folders(platform='msw', test=False):
    """Returns a sorted list of folders to search for C++-files for
    inclusion in the Faint-build. Expressed relative to the Faint root
    folder.

    """

    assert(platform in ('linux', 'msw'))
    folders = [
        "",
        "app/",
        "bitmap/",
        "commands/",
        "editors/",
        "formats/",
        "formats/bmp",
        "formats/gif",
        "formats/wx",
        "generated/",
        "generated/python/settings",
        "generated/resource",
        "geo/",
        "gui/",
        "gui/paint-dialog",
        "gui/dialogs",
        "objects/",
        "python/",
        "rendering/",
        "tablet/",
        "tasks/",
        "tools/",
        "text/",
        "util/",
        "util-wx/",

    ]
    if platform == 'msw':
        folders.extend((
            "tablet/msw",
            "tablet/msw/wintab"))
    if test:
        folders.append("tests")
        folders.append("test/gen")
        folders.append("test-sys")
        folders.append("tests/test-util")
    return sorted(folders)


class target_faint:
    objs_folder_prefix = "objs-bench"
    executable = "faint"


class target_image_test:
    objs_folder_prefix = "objs-image-test"
    source_folder = "tests/image-tests"
    executable = "tests/run-image-tests"


class target_unit_test:
    objs_folder_prefix = "objs-test" # Fixme: Change to objs-unit-tests
    source_folder = "tests/unit-tests"
    executable = "tests/run-unit-tests"


class target_benchmark:
    objs_folder_prefix = "objs-bench"
    source_folder = "tests/benchmarks"
    executable = "tests/run-benchmarks"


class target_gui_test:
    objs_folder_prefix = "objs-gui-test"
    source_folder = "tests/gui-tests"
    executable = "tests/run-gui-tests"
