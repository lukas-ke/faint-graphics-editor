#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import Bitmap, Pattern

class TestPattern(unittest.TestCase):

    def test_init(self):
        bmp = Bitmap((10,10), (255,0,255));
        p = Pattern(bmp)
