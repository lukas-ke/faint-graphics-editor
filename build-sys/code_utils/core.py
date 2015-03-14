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

def excluded(item, exclude_list):
    for exclude in exclude_list:
        if exclude in item:
            return True
    return False

def enumerate_files(startDir, extensions, excluded_dirs=None):
    if excluded_dirs is None:
        excluded_dirs = []

    fileList = []
    for root, dirs, files in os.walk(startDir):
        for item in dirs:
            if item.startswith(".") or excluded(item, excluded_dirs):
                dirs.remove(item)
        for file in files:
            for ext in extensions:
                if file.endswith(ext):
                    fileList.append(os.path.join(root, file))
                    break
    return fileList

def get_root_dir():
    root_dir = sys.argv[1] if len(sys.argv) > 1 else "../../"
    root_dir = os.path.abspath(root_dir)
    return root_dir
