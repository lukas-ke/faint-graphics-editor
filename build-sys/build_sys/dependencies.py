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

def find_includes(filename):
    import re
    with open(filename) as f:
        return re.findall(r'#include "(.*)"', f.read())

def find_header_dependencies(root, files):
    deps = {}
    for filename in files:
        includes = find_includes(filename)
        for include in [inc for inc in [root + incl for incl in includes] if
                        inc in files]:
            if include not in deps:
                deps[include] = set()
            deps[include].add(filename)
    return deps


def follow(include, deps, items, flat, done):
    for includer in deps.get(include,[]):
        if includer.endswith(".cpp"):
            items.add(includer)
        elif includer.endswith(".hh"):
            if includer in done:
                items.update(flat[includer])
            else:
                follow(includer, deps, items, flat, done)


def flatten(deps):
    flat={}
    done = set()
    for include in deps:
        if include not in flat:
            flat[include] = set()
        for includer in deps[include]:
            if includer.endswith("cpp"):
                flat[include].add(includer)
            elif includer.endswith(".hh"):
                follow(include, deps, flat[include], flat, done)
        done.add(include)
    return flat


def find_header_dependencies_all(root, src_folders):
    return find_header_dependencies(root,
                                    enumerate_all_sources(root, src_folders))


def enumerate_all_sources(root, src_folders):
    src = []
    for folder in src_folders:
        for file in [file for file in os.listdir(os.path.join(root,folder)) if
                     (file.endswith('cpp') or
                     file.endswith('.hh')) and not file.startswith('.') ]:
            src.append(os.path.join(root, folder, file).replace("\\","/"))
    return src


def get_flat_header_dependencies(root, src_folders):
    return flatten(find_header_dependencies_all(root, src_folders))


def _mapped_includes(root):
    """Return a dictionary mapping each file to all headers it includes."""
    files = enumerate_all_sources(root)
    includes = {}
    for filename in files:
        adj_filename = filename.replace(root, "")
        if adj_filename.startswith("/"):
            adj_filename = adj_filename[1:]
        includes[adj_filename] = find_includes(filename)
    return includes
