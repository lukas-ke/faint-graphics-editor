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

import faint
import os

def erase_columns(x0, x1):
    """Reduces the image width by removing all pixel columns from x0
    to x1 (inclusive).

    """

    x0, x1 = min(x0, x1), max(x0, x1)
    if x0 < 0:
        raise ValueError("Negative column specified.")

    image = faint.get_active_image()

    full = image.get_bitmap()
    w, h = full.get_size()
    w2 = w - (x1 - x0 + 1)
    if w2 <= 0:
        raise ValueError("Can't erase all columns.")

    right = full.subbitmap(x1 + 1, 0, w - x1 - 1, h)
    image.blit((x0,0),right)
    image.set_rect(0,0, w2, h)


def erase_rows(y0, y1):
    """Reduces the image height by removing all pixel rows from y0
    to y1 (inclusive).

    """
    y0, y1 = min(y0, y1), max(y0, y1)
    if y0 < 0:
        raise ValueError("Negative row specified.")

    image = faint.get_active_image()

    full = image.get_bitmap()
    w, h = full.get_size()
    h2 = h - (y1 - y0 + 1)
    if h2 <= 0:
        raise ValueError("Can't erase all rows.")

    bottom = full.subbitmap(0, y1 + 1, w, h - y1 - 1)
    image.blit((0, y0),bottom)
    image.set_rect(0,0, w, h2)


def erase_selection():
    """Removes the columns or rows indicated by the raster selection in
    the active image and shrinks the image.

    The raster selection must extend either across all rows or columns
    of the image.

    """

    image = faint.get_active_image()
    x, y, w, h = image.get_selection()
    img_w, img_h = image.get_size()

    if h == img_h:
        erase_columns(x, x + w)
        image.set_selection(0,0,0,0)
        image.command_name = "Erase Columns"

    elif w == img_w:
        erase_rows(y, y + h)
        image.set_selection(0,0,0,0)
        image.command_name = "Erase Rows"


def snowman():
    """Adds a text object with the snowman-character to test unicode
    support.

    """
    s = faint.get_settings()
    s.fontsize = 48
    preferred_font = "DejaVu Sans"
    if preferred_font in faint.list_fonts():
        s.font = preferred_font

    return faint.Text((20,20,100,100), '\u2603', s)


def scroll_traverse():
    """Scrolls through the image column by column, using
    scroll_page_down and scroll_page_right, eventually wrapping back
    to 0,0"""

    active = faint.get_active_image()
    max = active.get_max_scroll()
    current = active.get_scroll_pos()
    if current[1] >= max[1]:
        if current[0] >= max[0]:
            active.set_scroll_pos(0,0)
        else:
            active.set_scroll_pos(current[0], 0)
            active.scroll_page_right()
    else:
        active.scroll_page_down()


def _open_relative(image, replace, get_new_index):
    if image is None:
        image = faint.get_active_image()
    path = image.get_filename()
    if path is None:
        return

    d, f = os.path.split(path)
    files = sorted(os.listdir(d))

    i = files.index(f)
    new_index = get_new_index(i)
    if new_index < len(files):
        new_image = faint.app.open(os.path.join(d, files[new_index]))
        if replace and new_image is not None:
            faint.app.close(image)


def open_next(image=None, replace=False):
    _open_relative(image, replace, lambda i: i + 1)


def open_prev(image=None, replace=False):
    _open_relative(image, replace, lambda i: i - 1)


def open_first(image=None, replace=False):
    _open_relative(image, replace, lambda i: 0)


def open_last(image=None, replace=False):
    _open_relative(image, replace, lambda i: -1)
