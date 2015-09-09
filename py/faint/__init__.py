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

"""faint

A package of functions and classes for faint-graphics-editor.
"""

def __expose_built_ins(ifaint):
    """Make certain methods available like functions."""

    def expose(module, methods, obj):
        for item in methods:
            module.__dict__[item] = obj.__getattribute__(item)

    # Add all Faint global functions to the ifaint module
    #
    # ... these are actually methods on an instantiated (built-in) object in
    # order to access Faint-state, but are meant to be accessible like
    # functions.
    # Fixme: This should probably be part of faint/__init__.py
    expose(ifaint,
           [m for m in dir(ifaint.fgl) if not m.startswith("__")],
           ifaint.fgl)

    expose(ifaint,
           [m for m in dir(ifaint.active_settings) if not m.startswith("__")],
           ifaint.active_settings)

    # For backwards-compatibility with Faint <= 0.23, add some methods from
    # "app" which were previously global.
    APP_METHODS = ["add_format", "swap_colors", "update_settings"]
    APP_METHODS.extend([m for m in dir(ifaint.app) if m.startswith("tool")])
    expose(ifaint, APP_METHODS, ifaint.app)

try:
    import ifaint
    __expose_built_ins(ifaint)
    # Make all built-ins available under the faint module
    from ifaint import *
except ImportError as e:
    # When running simple API tests outside faint, without the
    # extension module, ifaint will not be available.
    pass

from faint.image import Pimage, PimageList, one_color_bg

def settings(**kwArgs):
    """Initialize settings with keyword arguments, e.g.
    settings(linewidth=1, brushsize=22).

    """
    s = Settings()
    for key in kwArgs:
        s.__setattr__(key, kwArgs[key])
    return s
