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

def _create_include_guard(file_path):
    file_name = os.path.split(file_path)[1]
    suffix = file_name.upper().replace(".", "_").replace("-","_")
    if suffix.startswith("FAINT_"):
        # Files which begin with faint- should not
        # require the include guard to start with FAINT_FAINT
        return suffix
    else:
        return "FAINT_" + suffix

def _check_include_guards(file_path):
    expected_guard = _create_include_guard(file_path)
    f = open(file_path)
    text = f.read()
    f.close()

    if text.find("#ifndef %s" % expected_guard) == -1:
        print("%s: Invalid include guard" % file_path)
        print("Expected: %s" % expected_guard)
    return expected_guard

if __name__ == '__main__':
    root_dir = get_root_dir()
    include_guards = []

    for file_path in enumerate_files(root_dir, (".hh",)):
        if file_path.find("msw_warn.hh") != -1:
            continue
        include_guard = _check_include_guards(file_path)
        if include_guard in include_guards:
            print("%s: Expected include guard collision" % file_path)
        include_guards.append(include_guard)
