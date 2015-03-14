#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2012 Lukas Kemmer
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

"""Helper utilities for Faint file formats."""

import ifaint

def _io_save_error(io_error, file_name):
    """Turns an IOException (which would be reported as an
    internal error) into an ifaint.SaveError, which is reported
    without stack-trace etc."""
    err_str = 'Faint could not write to "%s".\n\nError code: %d (%s).' % (
        file_name, io_error.errno, io_error.strerror)
    return ifaint.SaveError(err_str)


def _io_load_error(io_error, file_name):
    """Load equivalent of _io_save_error"""
    err_str = 'Faint could not read from "%s".\n\nError code: %d (%s).' % (
        file_name, io_error.errno, io_error.strerror)
    return ifaint.LoadError(err_str)


def open_for_writing_text(file_path):
    """Returns a File object or raises ifaint.SaveError"""
    try:
        f = open(file_path, 'w')
        return f
    except IOError as e:
        raise _io_save_error(e, file_path)


def open_for_writing_binary(file_path):
    """Returns a File object or raises ifaint.SaveError"""
    try:
        f = open(file_path, 'wb')
        return f
    except IOError as e:
        raise _io_save_error(e, file_path)

def open_for_reading_binary(file_path):
    """Returns a File object or raises ifaint.LoadError"""
    try:
        f = open(file_path, 'r')
        return f
    except IOError as e:
        raise _io_load_error(e, file_path)
