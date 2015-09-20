
#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import os

from faint import Bitmap, blit, Pattern
import faint
import py_ext_tests

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

    def test_blit(self):
        red = (255, 0, 0, 255)
        yellow = (255, 255, 0, 128)
        bmp1 = Bitmap((20, 10), red)
        bmp2 = Bitmap((10, 5), yellow)
        blit(bmp2, (1, 1), bmp1)
        self.assertEqual(bmp1.get_pixel(6,0), red)
        self.assertEqual(bmp1.get_pixel(6,1), yellow)
        self.assertEqual(bmp1.get_pixel(1,3), yellow)
        self.assertEqual(bmp1.get_pixel(10,3), yellow)
        self.assertEqual(bmp1.get_pixel(6,5), yellow)
        self.assertEqual(bmp1.get_pixel(6,6), red)

        # Self-blit, offset 1,1-pixels
        blit(bmp1, (1, 1), bmp1)
        self.assertEqual(bmp1.get_pixel(1, 3), red)
        self.assertEqual(bmp1.get_pixel(2, 3), yellow)

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
        b1.set_threshold((0.2, 0.8), (255, 0, 255), (0, 255, 255))
        b1.quantize()
        b1.pixelize(2)
        b1.erase_but_color((255,255,255), (255, 0, 255))
        b1.replace_alpha((100, 100, 100))
        b1.set_alpha(10)
        b1.color_balance((0,100), (0,100), (0,100))
        b1.line((0,0,10,10), (255,0,0))

        b1.boundary_fill((2, 3), (0,0,0), (255,0,0))
        b1.boundary_fill((2, 3), Pattern(Bitmap((10,10))), (255,0,0))

    def test_draw_objects(self):
        out_dir = py_ext_tests.make_test_dir(self)
        b1 = Bitmap((100, 100))
        b1.draw_objects([faint.create_Ellipse((0,0, 10, 10)),])
        faint.write_png(b1, os.path.join(out_dir, "b.png"), faint.png.RGB)

    def test_equal(self):
        b1 = Bitmap((10, 10))
        b2 = Bitmap((10, 10))
        self.assertEqual(b1, b2)

        b1.set_pixel((1,3), (255,0,0))
        self.assertNotEqual(b1, b2)

        b2.set_pixel((1,3), (255,0,0))
        self.assertEqual(b1, b2)

        b3 = Bitmap((10,20))
        b3.set_pixel((1,3), (255,0,0))
        self.assertNotEqual(b1, b3)

    def test_encode_bitmap_png(self):
        b1 = Bitmap((10, 10))
        b1.line((0,0,10,10), (255,0,255))

        encoded = faint.encode_bitmap_png(b1)
        b2 = faint.bitmap_from_png_string(encoded)

        self.assertEqual(b1, b2);
