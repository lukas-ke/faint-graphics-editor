#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
import os
import py_ext_tests

class TestPng(unittest.TestCase):

    def test_write_png(self):
        out_dir = py_ext_tests.make_test_dir(self)

        b1 = faint.Bitmap((5,7))
        b1.set_pixel((0,0),(255,0,255))

        fn = os.path.join(out_dir, "b1.png")
        faint.write_png(b1, fn, 0)

        b2, tEXt = faint.read_png(fn)
        self.assertEqual(b2.get_size(), (5,7))
        self.assertEqual(tEXt, {})


    def test_bad_args(self):
        with self.assertRaises(TypeError):
            faint.write_png("not a bitmap", py_ext_tests.make_test_dir(self), 0)
