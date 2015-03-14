#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2012 Lukas Kemmer
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

class XObject:
    """XObject dictionary and stream"""
    def __init__(self, data, width, height):
        self.dictionary = {"Type": "/XObject",
                           "Subtype": "/Image",
                           "BitsPerComponent" : "8",
                           "ColorSpace": "/DeviceRGB",
                           "Width": str(width),
                           "Height": str(height)}
        self.raw_data = data

    def data(self):
        return "stream\n" + self.raw_data + "\nendstream\n"

    def keys(self):
        k = list(self.dictionary.keys())
        k.extend(["Length"])
        return k

    def __getitem__(self, item):
        if item == "Length":
            return len(self.raw_data)
        return self.dictionary[item]

    def __len__(self):
        return len(self.dictionary.keys()) + 1 # Length
