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


def collapsed_faint(file_name, guard):
    """Returns the include-guard with a double FAINT_-prefix reduced to
    one.

    This allow include guards for files named "faint-" to start with
    a single "FAINT_"  prefix."""

    if file_name.startswith("faint-") and guard.startswith("FAINT_FAINT_"):
        return guard[6:]
    else:
        return None


def create_include_guard_id(file_path):
    """Returns a list of valid include guard identifiers for the given
    path.

    """

    if file_path.find("test-util") != -1:
        PREFIX = "FAINT_TEST_"
    else:
        PREFIX = "FAINT_"

    file_name = os.path.split(file_path)[1]
    guard_id = PREFIX + file_name.upper().replace(".", "_").replace("-","_")

    collapsed = collapsed_faint(file_name, guard_id)
    if collapsed is not None:
        return (guard_id, collapsed)
    return (guard_id,)


def include_guard(guard_id):
    return "#ifndef %s" % guard_id


def find_one_off_guard(text, file_path, guard_id):
    file_name = os.path.split(file_path)[1]
    g = ("#ifdef %s\n"
         "#error %s included twice\n"
         "#else\n"
         "#define %s\n"
         "#endif")
    g = g % (guard_id, file_name, guard_id)
    if text.find(g) != -1:
        return True
    return False


def find_normal_guard(text, guard_id):
    return text.find(include_guard(guard_id)) != -1


def find_guard_define(text, file_path, guard_ids):
    for guard_id in guard_ids:
        if find_normal_guard(text, guard_id):
            return guard_id
        elif find_one_off_guard(text, file_path, guard_id):
            return guard_id
    return None


def check_include_guards(file_path):
    """Prints a message if the include guard in the file does not match
    the expected include guard.

    """
    with open(file_path) as f:
        text = f.read()

    guard_ids = create_include_guard_id(file_path)
    used_id = find_guard_define(text, file_path, guard_ids)
    if used_id is not None:
        return used_id
    else:
        print("%s: Invalid include guard" % file_path)
        print("Expected: %s" % " or ".join(guard_ids))
        return None


def ignored(file_path):
    parts = ["test-sources",
             "generated", # Fixme: Consider including

             # The various test categories (e.g. unit tests, benchmarks) have
             # variants of this file. Just ignore them for these checks.
             "gen/defines.hh",
    ]
    for part in parts:
        if file_path.find(part) != -1:
            return True
    return False

if __name__ == '__main__':
    root_dir = get_root_dir()
    include_guards = []

    for file_path in enumerate_files(root_dir, (".hh",)):
        if ignored(file_path.replace("\\", "/")):
            continue
        guard_id = check_include_guards(file_path)
        if guard_id is not None:
            if guard_id in include_guards:
                print("%s: Expected include guard collision" % file_path)
            include_guards.append(guard_id)
