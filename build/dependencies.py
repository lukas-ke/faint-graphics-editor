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
import sys
import faint_info
import subprocess

build_dir = os.path.split(os.path.realpath(__file__))[0]
os.chdir(build_dir) # Fixme: Don't change dir, use absolute paths.
root_dir = os.path.split(build_dir)[0]

sys.path.append(os.path.join(root_dir, "build-sys"))
import build_sys.dependencies as dependencies
import build_sys.gv as gv

def unflat_header_dependencies(root, count_only, headers_only):
    """Prints a list of every header mapped to every file that directly
    includes the header.

    """

    deps = dependencies.find_header_dependencies_all(root,
        faint_info.get_src_folders())
    print("Header->direct dependents!")
    print()
    if not count_only:
        for dep in sorted(deps.keys(), key=lambda x:-len(deps[x])):
            num_deps = len(deps[dep])
            print("%s (%d):" % (dep.replace(root, ""), num_deps))
            for dependent in sorted(deps[dep]):
                if not headers_only or dependent.endswith(".hh"):
                    print(" ", dependent.replace(root, ""))
            print()
    else:
        for dep in sorted(deps.keys(), key=lambda x:-len(deps[x])):
            print(dep, len(deps[dep]))


def flat_header_dependencies(root, count_only):
    """Prints a list of headers mapped to every file that sees that
    header."""

    deps = dependencies.get_flat_header_dependencies(root,
        faint_info.get_src_folders())
    print("Header->dependents (recursive)")
    print()
    for dep in sorted(deps.keys(), key=lambda x:-len(deps[x])):
        num_deps = len(deps[dep])
        print("%s (%d):" % (dep.replace(root, ""), num_deps))
        if not count_only:
            for dependent in sorted(deps[dep]):
                print(" ", dependent.replace(root, ""))


def who_includes_graph(root, name):
    include_dict = dependencies.find_header_dependencies_all(root,
        faint_info.get_src_folders())
    dot_name = os.path.basename(name).replace(".hh", ".dot")
    png_name = dot_name.replace(".dot", ".png")
    with open(dot_name, 'w') as f:
        f.write(gv.who_includes(include_dict, name))

    out = open(png_name, 'wb')
    cmd = "dot -Tpng %s" % dot_name
    subprocess.call(cmd, stdout=out)

if __name__ == '__main__':
    root = faint_info.FAINT_ROOT
    if not root.endswith("/"):
        root += "/"

    count_only = '--count' in sys.argv
    if '--flat' in sys.argv:
        flat_header_dependencies(root, count_only)
    elif '--gv' in sys.argv:
        who_includes_graph(root, sys.argv[1])
    else:
        headers_only = '--hh' in sys.argv
        unflat_header_dependencies(root, count_only, headers_only)
