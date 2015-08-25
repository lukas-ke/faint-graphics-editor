#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
# Fixme: inconsistent: parse_[svg]_string vs parse_doc
# Should probably remove "_svg"
# Also: Would be nicer to lift these into faint.svg via __init__.py
from faint.svg.parse_svg import parse_svg_string, parse_doc

SVG = """\
<svg xmlns:svg="http://www.w3.org/2000/svg"
xmlns="http://www.w3.org/2000/svg" version="1.0"
width="111" height="222">
  <rect x="0" y="0" width="100" height="100"/>
</svg>"""

class TestSVG(unittest.TestCase):

    def test_parse_svg_string(self):
        props = faint.ImageProps()
        parse_svg_string(SVG, props)
        frame = props.get_frame(0)
        image = faint.Image(frame)

        self.assertEqual(image.num_objects(), 1)
        self.assertEqual(image.get_size(), (111, 222))
        self.assertEqual(image.color_count(), 1)
