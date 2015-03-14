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
import sys

def _heading(text):
    print("-" * 65)
    print(text)
    print("-" * 65)

def _print_dict(d, heading, emacs_mode):
    _heading(heading)
    count = 0
    for filename in d:
        count += len(d[filename])
    print("Files: %d Items: %d" % (len(d), count))


    for filename, items in sorted(d.items(), key=lambda x: len(x[1])):
        print()
        filename = filename.replace("\\", "/")
        if not emacs_mode:
            print(filename)
        for line_num, text in items:
            if emacs_mode:
                print("%s(%d) : error C0000: %s" % (filename, line_num, text.strip()))
            else:
                print("%d: %s" % (line_num, text.strip()))


def _find_in_file(filename, strings):
    f = open(filename)
    lines = f.readlines()
    f.close()
    fixme_lines = []
    for string in strings:
        fixme_lines.append([])

    for num, line in enumerate(lines):
        for list_num, string in enumerate(strings):
            if line.lower().find(string) != -1:
                fixme_lines[list_num].append((num + 1,line))
                break
    return fixme_lines

if __name__ == '__main__':
    root_dir = get_root_dir()
    emacs_mode = '--emacs' in sys.argv

    file_to_fixmes = {}
    file_to_cpp = {}
    for filename in enumerate_files(root_dir, (".hh",".cpp")):
        fixmes, cpp = _find_in_file(filename, ("fixme", "c++1"))
        if len(fixmes) > 0:
            file_to_fixmes[filename] = fixmes
        if len(cpp) > 0:
            file_to_cpp[filename] = cpp

    py_file_to_fixmes = {}
    for filename in enumerate_files(root_dir, (".py",),
                                    excluded_dirs=('pythonbundle', 'code_utils')):
        py_fixmes, = _find_in_file(filename, ("fixme",))
        if len(py_fixmes) > 0:
            py_file_to_fixmes[filename] = py_fixmes

    _print_dict(file_to_fixmes, "Fixmes ('C++)", emacs_mode);
    print()
    _print_dict(file_to_cpp, "C++-conformance", emacs_mode)
    print()
    _print_dict(py_file_to_fixmes, "Fixmes ('Python)", emacs_mode)
