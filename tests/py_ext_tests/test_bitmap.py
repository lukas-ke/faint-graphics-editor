#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import Bitmap, blit

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


    def test_fill(self):
        bmp = Bitmap((10,10))
        bmp.fill((9,9), (255, 0, 255))
        self.assertEqual(bmp.get_pixel(3, 3), (255,0,255,255))


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


    def test_subbitmap(self):
        bmp1 = Bitmap((10, 10))
        bmp2 = bmp1.subbitmap((1, 1, 3, 3))
        self.assertEqual(bmp2.get_size(), (3,3))

    def test_api(self):
        """Some haphazard API calls with too few assertions.

        These should be distributed to functions and tested more
        thoroughly, but they may at least catch API changes for now...

        """

        b1 = Bitmap((10,5))
        b2 = Bitmap((5,10))

        b1.aa_line((0, 0, 10, 20), (255, 0, 255))
        blit(b1, (2, 2), b2)

        b1.clear((255,255,0))
        self.assertEqual(b1.color_count(), 1)

        b1.set_pixel((2, 2), (0, 255, 255))
        self.assertEqual(b1.color_count(), 2)

        b1.desaturate()
        b1.desaturate_weighted()
        b1.flip_horizontally()
        b1.flip_vertically()
        b1.gaussian_blur(2.5)
        b1.invert()
        b1.replace_color((0, 0, 255, 200), (255, 0, 0, 255))
        b1.rotate(3.14, (255, 0, 255))
        b1.sepia(1.0)
        b1.set_threshold(0, 255, (255, 0, 255), (0, 255, 255))
        b1.quantize()
        b1.pixelize(2)
        b1.erase_but_color((255,255,255), (255, 0, 255))
        b1.replace_alpha((100, 100, 100))
        b1.set_alpha(10)
        b1.color_balance((0,100), (0,100), (0,100))

        # Fixme: Add boundary fill
        # Fixme: bmp.line(0,0,10,10) # Fixme: Removed
        # Fixme: Paste
