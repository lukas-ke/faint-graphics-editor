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

import faint
from . create import create_arrowhead

class SvgBuildState:
    """Keeps track of what defs have been generated during SVG
    creation."""

    def __init__(self):
        self.linear_gradients = []
        self.radial_gradients = []
        self.patterns = []
        self.arrows = {} # color to arrowhead-marker-elements

        # Maps Faint fill types to their list and id-prefix
        self.class_map = {
            faint.LinearGradient: ('lgradient', self.linear_gradients),
            faint.Pattern: ('pattern', self.patterns),
            faint.RadialGradient: ('rgradient', self.radial_gradients)}

    def add_arrowhead(self, color):
        """Adds a marker for an arrowhead of the given color. Returns the
        marker id, for referencing.

        """

        def arrowhead_id(num):
            return 'Arrowhead_{num}'.format(num=num)

        if color in self.arrows:
            # Reuse old id
            num, element = self.arrows[color]
            return arrowhead_id(num)

        # Add new element
        num = len(self.arrows)
        marker_id = arrowhead_id(num)
        self.arrows[color] = (num, create_arrowhead(marker_id, color))
        return marker_id

    def get_marker_elements(self):
        """Returns a list of marker elements"""
        # Fixme: Currently only arrowheads
        return [item[1] for item in self.arrows.values()]

    def get_items(self):
        """Returns all def-entries with their id as a list of tuples"""
        combined_list = []
        for prefix, item_list in self.class_map.values():
            combined_list.extend(zip(self._get_id_range(prefix, len(item_list)),
                                 item_list))
        return combined_list

    def get_defs_id(self, item):
        """Returns the id for the specified item in the <defs>-element for
        linking.

        """
        prefix, item_list = self.class_map[item.__class__]
        return self._get_id(item, prefix, item_list)

    def should_link(self, item):
        """True if the passed in item should be linked to in the <defs> or
        used inline.

        """
        return item.__class__ in self.class_map.keys()

    def _get_id(self, item, prefix, item_list):
        """Get the id for the passed in item. Inserts the item and returns a
        new id if not yet managed.

        """
        try:
            index = item_list.index(item)
        except ValueError:
            index = len(item_list)
            item_list.append(item)

        return self._id_for_index(prefix, index)

    def _get_id_range(self, prefix, item_count):
        # Fixme: Remove?
        return [self._id_for_index(prefix, i) for i in range(item_count)]

    @staticmethod
    def _id_for_index(prefix, index):
        """Generates an id for a defs item."""
        return "%s%d" % (prefix, index + 1)
