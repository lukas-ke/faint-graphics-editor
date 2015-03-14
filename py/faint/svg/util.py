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

"""Helper functions for svg-parsing and writing.

Note: To simplify testing, this file should not depend on ifaint."""

from math import tan, cos, pi, sin, atan2

_DEG_PER_RAD = 360 / (2 * pi)
_RAD_PER_DEG = 1 / _DEG_PER_RAD

def arrow_line_end(arrowTipX, arrowTipY, angle, lineWidth):
    x = arrowTipX + cos(angle) * 15 * (lineWidth / 2.0)
    y = arrowTipY + sin(angle) * 15 * (lineWidth / 2.0)
    return x, y

def rad2deg(angle):
    """Converts angle from radians to degrees."""
    return angle * _DEG_PER_RAD

def rad_angle( x0, y0, x1, y1 ):
    return atan2( y1 - y0, x1 - x0 )

def deg2rad(angle):
    """Converts angle from degrees to radians."""
    return angle * _RAD_PER_DEG
