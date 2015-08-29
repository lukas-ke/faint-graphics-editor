#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
# Fixme: Would be nicer to lift the parse functions faint.svg via __init__.py
import faint.svg.parse_svg as svg

SVG = """\
<svg xmlns:svg="http://www.w3.org/2000/svg"
xmlns="http://www.w3.org/2000/svg" version="1.0"
width="111" height="222">
  <rect x="10" y="20" width="101" height="102"/>
</svg>"""

class TestParseSVG(unittest.TestCase):

    def test_parse_ImageProps(self):
        props = faint.ImageProps()
        svg.from_string(SVG, props)
        frame = props.get_frame(0)
        image = faint.Image(frame)

        self.assertEqual(image.num_objects(), 1)
        self.assertEqual(image.get_size(), (111, 222))
        self.assertEqual(image.color_count(), 1)

    def test_parse_PimageList(self):
        l = faint.PimageList()
        svg.from_string(SVG, l)
        self.assertEqual(len(l.frames), 1)

        f = l.frames[0]
        self.assertEqual(len(f.objects), 1)

        o = f.objects[0]
        self.assertAlmostEqual(o.rect(),
            (10.0, 20.0, 101.0, 102.0))
