#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
import faint.svg.write_svg as write_svg

class TestWriteSVG(unittest.TestCase):

    def test_pimage(self):
        l = faint.PimageList()
        l.add_frame((640,480))
        faint.one_color_bg(l)

        print(write_svg.to_string(l))
