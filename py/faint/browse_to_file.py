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
import faint

"""Functionality for showing the opened image file in the system file
browser.

"""

def file_browse_cmd_msie(file_path):
    if os.path.exists(file_path):
        return 'explorer /select,"%s"' % file_path

    # The file is missing (probably renamed or moved), explore
    # the directory instead, if it exists
    dir_path = os.path.dirname(file_path)
    if os.path.exists(dir_path):
        return u'explorer "%s"' % dir_path
    else:
        return ""


def file_browse_cmd_thunar(file_path):
    return ('thunar', os.path.dirname(f))


def  file_browse_command_nautilus(file_path):
    return ('nautilus', os.path.dirname(f))


# Function used for producing the arguments for subprocess.Popen for
# browsing to a file.
#
# The return value is used for subprocess.Popen, so this should return
# a list or a string (list is preferable, except on Windows, see
# http://docs.python.org/2/library/subprocess.html#converting-argument-sequence
external_browse_to_file_command = None


if os.name == 'nt':
    external_browse_to_file_command = file_browse_cmd_msie


def browse_to_active_file():
    """Calls external_browse_to_file_command with the filename of the
    active image."""

    if external_browse_to_file_command is None:
        raise ValueError("external_browse_to_file_command not configured.")

    filename = faint.get_active_image().get_filename()
    if filename is not None:
        import subprocess
        cmd_string = external_browse_to_file_command(filename)
        if len(cmd_string) != 0:
            cmd = external_browse_to_file_command(filename)
            subprocess.Popen(cmd, shell=True)
