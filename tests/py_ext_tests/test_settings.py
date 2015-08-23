#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint
from faint import Settings

class TestSettings(unittest.TestCase):

    def test_empty_settings(self):
        s = Settings()
        self.assertEqual(str(s), '')

        with self.assertRaises(ValueError):
            s.linewidth


    def test_set(self):
        s = Settings()
        s.linewidth = 10
        self.assertEqual(s.linewidth, 10)


    def test_copy(self):
        from copy import copy

        s1 = Settings()

        s1.linewidth = 10
        s2 = copy(s1)
        self.assertEqual(s2.linewidth, 10)

        s2.linewidth = 20
        self.assertEqual(s1.linewidth, 10)
        self.assertEqual(s2.linewidth, 20)


    def test_update(self):
        s1 = Settings()
        s1.linewidth = 10

        s2 = Settings()
        s2.brushsize = 23

        s1.update(s2)
        self.assertEqual(s1.linewidth, 10)

        with self.assertRaises(ValueError):
            v = s1.brushsize

        s1.update_all(s2)
        self.assertEqual(s1.brushsize, 23)

        s1.update_all(s1)
        self.assertEqual(s1.brushsize, 23)
        self.assertEqual(s1.linewidth, 10)


    def test_paint(self):
        s = Settings()
        s.fg = (255,0,0)
        self.assertEqual(s.fg, (255,0,0,255))

        def make_lg():
            return faint.LinearGradient((0.0,(128,0,55)))

        s.fg = make_lg()
        self.assertEqual(s.fg, make_lg())

        def make_rg():
            return faint.RadialGradient((0, 0), ((0.0, (128, 0, 55))))

        s.fg = make_rg()
        self.assertEqual(s.fg, make_rg())
        self.assertNotEqual(s.fg, make_lg())

        # Swap (Maybe I don't need to test this, but it's cool!)
        s.fg, s.bg = make_lg(), make_rg()
        s.fg, s.bg = s.bg, s.fg

        self.assertEqual(s.fg, make_rg())
        self.assertEqual(s.bg, make_lg())

        with self.assertRaises(TypeError):
            s.fg = 5

        s.fg = faint.Pattern(faint.Bitmap((10,5)))
        self.assertIsInstance(s.fg, faint.Pattern) # No pattern equality yet
