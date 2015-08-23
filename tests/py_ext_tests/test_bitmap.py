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
