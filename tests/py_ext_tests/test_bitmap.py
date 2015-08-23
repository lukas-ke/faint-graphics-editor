#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import Bitmap

class TestBitmap(unittest.TestCase):

    def test_size_init(self):
        bmp = Bitmap((123, 456))
        self.assertEqual(bmp.get_size(), (123, 456))
        self.assertEqual(bmp.get_pixel(1,1), (255, 255, 255, 255))


    def test_size_color_rgb_init(self):
        bmp = Bitmap((3, 5), (255, 0, 255))
        self.assertEqual(bmp.get_pixel(0, 0), (255, 0, 255, 255))


    def test_size_color_rgba_init(self):
        bmp = Bitmap((3, 5), (255,0,255, 50))
        self.assertEqual(bmp.get_pixel(1,1), (255,0,255,50))


    def test_set_pixel(self):
        bmp = Bitmap((10, 10))
        bmp.set_pixel((1,1),(255,0,255))
        self.assertEqual(bmp.get_pixel(1,1),(255,0,255,255))


    def test_copy(self):
        from copy import copy

        src = Bitmap((3, 4));
        pos, color = (2,1), (1,2,3,4)
        src.set_pixel(pos, color)

        dst = copy(src)

        # Copy is identical - at least one of the pixels :)
        self.assertEqual(dst.get_pixel(pos), color)

        other_color = 0, 255, 0, 0
        dst.set_pixel(pos, other_color)

        # Copy is detached from source
        self.assertEqual(src.get_pixel(pos), color)
