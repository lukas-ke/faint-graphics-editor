#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from copy import copy
import faint

class TestShape(unittest.TestCase):

    def test_init(self):
        shape = faint.Shape()
        self.assertEqual(shape.get_type(), "Rectangle");

    def test_copy(self):
        s1 = faint.Shape()
        s2 = copy(s1)

        self.assertNotEqual(s1, s2)
        self.assertEqual(s1.get_type(), "Rectangle")
        self.assertEqual(s2.get_type(), "Rectangle")
