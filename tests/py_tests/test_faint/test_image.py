#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import Image, FrameProps, ImageProps

class TestImage(unittest.TestCase):

    def test_init(self):
        # Fixme: Add proper Image constructor.
        p = ImageProps()
        f = p.add_frame(200, 300)
        i = Image(f)
        self.assertEqual(i.get_size(), (200, 300))
        self.assertEqual(i.color_count(), 1)

        i.set_pixel((0,0), (123,42,50))
        self.assertEqual(i.color_count(), 2)

        self.assertEqual(i.get_pixel(0,0), (123,42,50, 255))

        i.desaturate()
        self.assertEqual(i.get_pixel(0,0), (71,71,71, 255))
        self.assertEqual(i.color_count(), 2)
