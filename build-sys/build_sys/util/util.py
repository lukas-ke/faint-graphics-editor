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

def changed(source, target):
    """Returns true if target is missing or if source is modified since
    target."""
    return (not os.path.exists(target) or
            os.path.getmtime(target) < os.path.getmtime(source))


def any_changed(sources, target):
    """Returns true if target is missing or if any source is modified since
    target."""

    if not os.path.exists(target):
        return True

    target_time = os.path.getmtime(target)
    for source in sources:
        if target_time < os.path.getmtime(source):
            return True
    return False

def joined(root, folders):
    return [os.path.join(root, f) for f in folders]

def strip_ext(file_name):
    dot = file_name.find('.')
    assert(dot != -1)
    return file_name[:dot]


def create_include_guard(file_path):
    base_name = os.path.basename(file_path)
    return "FAINT_" + strip_ext(base_name).upper().replace('-','_') + "_HH"


def timed(func, *args, **kwArgs):
    """Times a function call"""
    import time
    t1 = time.time()
    res = func(*args, **kwArgs)
    t2 = time.time()
    print('  %s took %0.3f ms' % (func.__name__, (t2-t1)*1000.0))
    return res

def not_timed(func, *args, **kwArgs):
    return func(*args, **kwArgs)


def print_timing(func):
    """Times function calls, should be used as a decorator:
      > @print_timing
      > def some_func(): ...
    """

    def wrapper(*arg, **kwArgs):
        return timed(func, *arg, **kwArgs)
    return wrapper


def list_by_ext(folder, ext):
    return [file for file in os.listdir(folder) if file.endswith(ext)
            and not file.startswith('.')]


def list_hh(folder):
    return list_by_ext(folder, 'hh')


def list_cpp(folder):
    return list_by_ext(folder, 'cpp')
