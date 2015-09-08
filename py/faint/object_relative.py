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

"""Functions for getting the nearest object in a given direction
from another object."""

import faint

def next_left(obj):
    frame = faint.get_object_frame(obj)
    by_x = sorted(frame.get_objects(), key=lambda o: o.pos[0])
    i = by_x.index(obj)
    return by_x[i - 1]

def next_right(obj):
    frame = faint.get_object_frame(obj)
    by_x = sorted(frame.get_objects(), key=lambda o: o.pos[0])
    i = by_x.index(obj)
    return by_x[(i + 1) % len(by_x)]

def next_up(obj):
    frame = faint.get_object_frame(obj)
    by_y = sorted(frame.get_objects(), key=lambda o: o.pos[1])
    i = by_y.index(obj)
    return by_y[(i - 1)]

def next_down(obj):
    frame = faint.get_object_frame(obj)
    by_y = sorted(frame.get_objects(), key=lambda o: o.pos[1])
    i = by_y.index(obj)
    return by_y[(i + 1) % len(by_y)]

def next_behind(obj):
    frame = faint.get_object_frame(obj)
    by_z = frame.get_objects()
    i = by_z.index(obj)
    return by_z[i - 1]

def next_ahead(obj):
    frame = faint.get_object_frame(obj)
    by_z = frame.get_objects()
    i = by_z.index(obj)
    return by_z[(i + 1) % len(by_z)]
