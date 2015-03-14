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

import os
from core import enumerate_files, get_root_dir

def _filtered(includes):
    return [incl for incl in includes if "pythoninclude.hh" not in incl]

def strip_comments(line):
    comment_pos = line.rfind("//")
    if comment_pos != -1:
        return line[:comment_pos]
    return line

def _get_file(line):
    file_name = strip_comments(line)[len("#include "):].strip()
    if file_name.startswith('"'):
        return file_name[1:-1]
    return file_name

def _valid_interjection(line):
    return line == "\n" or line.startswith("#if") or line.startswith("#endif") or line.startswith("#define")

def _external_include(line):
    included = _get_file(line)
    return included.startswith('wx/') or included.startswith("<") or included == 'cairo.h' or included == "pango/pangocairo.h"

def _print_error(filename, num, message):
    print("%s:%d %s" % (filename, num, message))

def _incl_key(s):
    pos = s.rfind(".")
    if pos != -1:
        return s[:pos]
    return s

def _check_includes(filename, root_dir):
    f = open(filename)
    lines = f.readlines()
    include_lines = []
    external_include_lines = []
    non_include = None
    first_include = -1
    for num, line in enumerate(lines):
        if line.startswith("#include"):
            if first_include == -1:
                first_include = num
            if len(include_lines) != 0 and len(external_include_lines) != 0 and non_include != None:
                _print_error(filename, num + 1, "Unexpected content between includes" + non_include[1].strip())

            if _external_include(line):
                external_include_lines.append(line)
                if len(include_lines) != 0:
                    _print_error(filename, num + 1, "External include after internal" + line.strip())
            else:
                include_lines.append(line)
        elif len(include_lines) != 0 and len(external_include_lines) != 0 and not _valid_interjection(line):
            non_include = num, line

    filtered = _filtered(include_lines)
    if len(filtered) != 0:
        sorted_includes = sorted(filtered, key=_incl_key)
        for num, (a, b) in enumerate(zip(filtered, sorted_includes)):
            if a != b:
                _print_error(filename, first_include + num, "Missorted includes, %s != %s" % (a, b))
                break

    for num, include in enumerate([_get_file(l) for l in include_lines]):
        full_path = os.path.join(root_dir, include)
        if not os.path.exists(full_path):
            _print_error(filename, first_include + num, "Invalid include: %s" % full_path)


if __name__ == '__main__':
    root_dir = get_root_dir()
    for filename in enumerate_files(root_dir, (".hh", ".cpp")):
        if filename.find("mainframe") == -1:
            _check_includes(filename, root_dir)
