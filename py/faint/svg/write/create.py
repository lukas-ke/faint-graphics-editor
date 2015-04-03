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

import xml.etree.ElementTree as ET

def create_arrowhead():
    """Creates a forward-pointing arrow-head marker."""
    element = ET.Element('marker')
    element.set('id', 'Arrowhead')
    element.set('markerUnits', 'strokeWidth')
    element.set('markerWidth', '7.5')
    element.set('markerHeight', '6.6')
    element.set('orient', 'auto')
    element.set('refX', '0')
    element.set('refY', '3.3') # Offset by half width

    path = ET.Element('path')
    path.set('d', "M 0 0 L 7.5 3.3 L 0 6.6 z")
    element.append(path)
    return element
