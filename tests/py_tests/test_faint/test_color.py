#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import os

from faint import Color

class TestColor(unittest.TestCase):
    """Tests the faint.Color class"""

    def test_color(self):
        # Default construction
        self.assertEqual(Color(), (0,0,0,255))
        self.assertNotEqual(Color(), (0,1,0,255))

        # Comparison
        self.assertEqual(Color(1,2,3), Color(1,2,3))
        self.assertEqual(Color(1,2,3), Color(1,2,3,255))

        # Indexing
        self.assertEqual(Color(0,1,2,3)[0], 0)
        self.assertEqual(Color(0,1,2,3)[1], 1)
        self.assertEqual(Color(0,1,2,3)[2], 2)
        self.assertEqual(Color(0,1,2,3)[3], 3)

        # Slicing
        self.assertEqual(Color(0,1,2,3)[0:0], ())
        self.assertEqual(Color(0,1,2,3)[0:1], (0,))
        self.assertEqual(Color(0,1,2,3)[0:2], (0,1))
        self.assertEqual(Color(0,1,2,3)[:], (0,1,2,3))
        self.assertEqual(Color(0,1,2,3)[:-1], (0,1,2))

        # Attributes
        self.assertEqual(Color(0,1,2,3).r, 0)
        self.assertEqual(Color(0,1,2,3).g, 1)
        self.assertEqual(Color(0,1,2,3).b, 2)
        self.assertEqual(Color(0,1,2,3).a, 3)
