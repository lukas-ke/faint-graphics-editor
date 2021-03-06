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
import re
import sys

EXTENSIONS = [
    ".cpp",
    ".hh",
    ".txt",
]

EXCLUDED_DIRS = [
    "__pycache__",
    "build",
    "build-sys",
    "code_utils",
    "doxygen",
    "help",
    "installer",
]

def format_filename(root_dir, f):
    f = f.replace(root_dir, "")
    return  f[1:]  if f[0] == "\\" else f

def write_defs_summary(file_name, defs):
    with open(file_name, 'w') as f:
        for d in sorted(defs.keys()):
            key = d
            path = defs[d][0]
            charNum = str(defs[d][2])
            label = defs[d][1]
            if len(label) == 0:
                label = key
            f.write(key + "---" +
                    path + "---" +
                    charNum + "---" +
                    label + "===")

def format_def_line(l):
    return l.strip().replace("// ", "")

def format_def_content(text):
    return "\n".join(format_def_line(l) for l in text.split("\n"))

def write_defs_index(file_name, defs):
    with open(file_name, 'w') as f:
        f.write("Index of definitions in Faint source files.\n")
        f.write("With faint-start-magic, the following entries\n")
        f.write("should be clickable links.\n\n")

        for key in sorted(defs.keys()):
            f.write("\\ref(%s) (%s)\n" % (key, key))

if __name__ == '__main__':
    root_dir = sys.argv[1]
    out_file = sys.argv[2]
    index_file = sys.argv[3]

    defs_pattern = re.compile(r"\\def\((.*?)\)(.*?);", re.DOTALL|re.MULTILINE)
    refs_pattern = re.compile(r"\\ref\((.*?)\)")

    defs = {}
    refs = {}

    for filename in enumerate_files(root_dir,
                                    extensions=EXTENSIONS,
                                    excluded_dirs=EXCLUDED_DIRS):
        with open(filename) as f:
            text = f.read()
            for entry in re.finditer(defs_pattern, text):
                name = entry.group(1)
                if name in defs:
                    print("Duplicate definition: %s" % name)
                else:
                    content = format_def_content(entry.group(2).strip())
                    defs[name] = [filename, content, entry.span()[0]]

            for entry in re.finditer(refs_pattern, text):
                name = entry.group(1)
                if name not in refs:
                    refs[name] = []
                refs[name].append(filename)


    print("Definitions:")
    for num, name in enumerate(sorted(defs.keys())):
        print(name + ": " + format_filename(root_dir, defs[name][0]))
        for f in refs.get(name, []):
            print(" " * len(name) + "  <- %s" % format_filename(root_dir, f))
        if num != len(defs) - 1:
            print()

    print()
    print("References:")
    for name in sorted(refs.keys()):
        for f in refs[name]:
            print(name + ": ", format_filename(root_dir, f))

    for name in defs.keys():
        if name not in refs:
            print("Unreferenced define: %s" % name)

    for name in refs.keys():
        if name not in defs.keys():
            print("Invalid reference: %s %s" % (name, refs[name][0]))

    write_defs_summary(out_file, defs)
    write_defs_index(index_file, defs)
