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

"""Functionality for expanding images so that features in the
different images are aligned, by anchoring around some point.

"""

import faint

def anchor(images, anchors):
    """Expands the images so that all the anchors (one per image) end
    up at the same x,y position in all images.

    Useful for example when comparing diagrams, so that the content
    align in all images regardless of caption sizes

    """
    assert(len(images) == len(anchors))
    assert(len(images) > 1)
    max_x = anchors[0][0]
    max_y = anchors[0][1]
    for p in anchors:
        max_x = max(p[0], max_x)
        max_y = max(p[1], max_y)

    for img, anchor in zip(images, anchors):
        w, h = img.get_size()
        dx = anchor[0] - max_x
        dy = anchor[1] - max_y
        if (dx != 0 or dy != 0):
            img.set_rect(dx, dy, w - dx, h - dy)


# Images to anchor-positions
_flagged = {}


def flagged():
    return copy.copy(_flagged)


def toggle_flag_pixel():
    """Adds the image and the current mouse position to a map of
    image-to-pixel. Indicates this with the set_point_overlay-function.

    If the mouse position is already flagged for the image, the flag
    is removed instead.

    The flagging can be used for to re-anchor images with anchor_flagged.

    """
    image = faint.get_active_image()
    pos = faint.get_mouse_pos()
    if image in _flagged and _flagged[image] == pos:
        del _flagged[image]
        image.clear_point_overlay()
    else:
        _flagged[image] = pos
        image.set_point_overlay(*pos)


def anchor_flagged():
    """Expands flagged images so that the flagged pixels are at the
    same x,y-position in all images.

    """

    images = [img for img in _flagged.keys() if not img.expired()]
    anchors = [_flagged[img] for img in images]
    anchor(images, anchors)
    clear_flagged()


def clear_flagged():
    for image in _flagged:
        if not image.expired():
            image.clear_point_overlay()
    _flagged.clear()
