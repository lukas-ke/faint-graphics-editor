#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from copy import copy
import faint

class TestShape(unittest.TestCase):

    def test_init(self):
        with self.assertRaises(TypeError):
            shape = faint.Shape()


    def test_copy(self):
        s1 = faint.create_Rect((0,0,100,100))
        self.assertEqual(s1.type, "Rectangle");
        s2 = copy(s1)

        self.assertNotEqual(s1, s2)
        self.assertEqual(s1.type, "Rectangle")
        self.assertEqual(s2.type, "Rectangle")

        dst = 20.0, 20.0
        s1.pos = dst

        self.assertEqual(s1.pos, dst)
        self.assertNotEqual(s2.pos, dst)
