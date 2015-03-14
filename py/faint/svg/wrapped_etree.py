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

"""Stand in for xml.etree.ElementTree which imports all its names and
wraps the parse and fromstring functions with expat-exception
conversion.

"""

from xml.etree.ElementTree import *
import faint.svg.expat_util as _expat
import ifaint as _ifaint

def _wrap_expat_exception(func):
    """Return the passed in function wrapped with a try-catch turning
    document-related expat exceptions into ifaint.LoadErrors, so that
    no parse stack trace is shown for errors in the SVG.

    """

    def wrapper(*args, **kwArgs):
        try:
            return func(*args, **kwArgs)
        except ParseError as e:
            if _expat.is_document_related(e):
                raise _ifaint.LoadError("Error in file:\n" + str(e))
            else:
                # Re-raise as internal error
                raise
        wrapper.__doc__ = func.__doc__

    return wrapper

parse = _wrap_expat_exception(parse)
fromstring = _wrap_expat_exception(fromstring)
