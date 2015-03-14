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

"""Function for converting raster text in the image to a string.
Requires that the tesseract-ocr application is available in the path.

See http://code.google.com/p/tesseract-ocr/

"""

import os
import subprocess as sp

import ifaint
from faint.extra.util import hide_console

__all__ = ["to_text"]

def to_text(temp_folder, image=None, language="eng"):
    """Uses tesseract-ocr to return a string of the raster text in the image. Uses
    the active image if image is None. Will leave some temporary files
    in the specified folder.

    """

    if image is None:
        image = ifaint.get_active_image()

    temp_png = os.path.join(temp_folder, "tesseract-in.png")
    temp_txt = os.path.join(temp_folder, "tesseract-out")
    image.save_backup(temp_png)

    cmd = ["tesseract", temp_png, temp_txt, "-l %s" % language]
    tsr = sp.Popen(cmd, startupinfo=hide_console())

    if tsr.wait() == 0:
        with open("%s.txt" % temp_txt, 'rb') as f:
            return f.read().decode("utf-8")
    else:
        raise ValueError("Failed running Tesseract.")
