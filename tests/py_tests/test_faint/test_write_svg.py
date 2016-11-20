#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
import faint.svg.write_svg as write_svg

# A previous result used as key kind of brittle, but OK for now.
SVG_KEY = """<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE svg PUBLIC "-//W3C//DTD SVG 1.1//EN"
  "http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd">
<svg height="480" version="1.1" width="640" xmlns="http://www.w3.org/2000/svg" xmlns:faint="http://www.code.google.com/p/faint-graphics-editor" xmlns:xlink="http://www.w3.org/1999/xlink"><defs><faint:grid dashed="False" enabled="False" spacing="40" x="0" y="0" /></defs><rect faint:background="1" fill="rgb(255, 255, 255)" height="100%" width="100%" x="0" y="0" /><polygon faint:type="rect" points="0.0, 0.0, 99.0, 0.0, 99.0, 99.0, 0.0, 99.0" style="fill:none;stroke:rgb(0, 0, 0);stroke-opacity:1.0;stroke-width:1.0;stroke-linejoin:miter;" /></svg>"""

class TestWriteSVG(unittest.TestCase):

    def test_pimage(self):
        self.maxDiff = None
        l = faint.PimageList()
        f = l.add_frame((640,480))
        f.Rect((0,0,100,100))

        s = write_svg.to_string(l)
        self.assertEqual(s, SVG_KEY)
