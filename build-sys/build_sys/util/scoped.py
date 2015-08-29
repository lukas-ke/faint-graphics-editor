#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2015 Lukas Kemmer
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

class working_dir:
    """Used with with-statement to change working directory in a scope."""

    def __init__(self, new_dir):
        self.new_dir = new_dir
        self.old_dir = os.getcwd()

    def __enter__(self):
        os.chdir(self.new_dir)

    def __exit__(self, type, value, traceback):
        os.chdir(self.old_dir)


class no_output:
    """Used with with-statement to suppress all output."""
    def __init__(self):
        self.stdout = sys.stdout
        self.stderr = sys.stderr
        self.devnull = open(os.devnull, 'w')
    def __enter__(self):
        sys.stdout = self.devnull
        sys.stderr = self.devnull

    def __exit__(self, type, value, traceback):
        sys.stdout = self.stdout
        sys.stderr = self.stderr
        self.devnull.close()
