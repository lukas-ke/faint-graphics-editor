#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
import faint.extra.dot as dot
from faint.image import Pimage, PimageList

class TestDot(unittest.TestCase):

    def test_dot(self):
        p = PimageList()

        dot.load_dot_string(b"digraph g {a->b;}\n", p)
        self.assertEqual(len(p.frames), 1)

        f = p.frames[0]
        object_types = ["Polygon",
                        "Ellipse",
                        "Text Region",
                        "Ellipse",
                        "Text Region",
                        "Path",
                        "Polygon"]
        self.assertEqual(len(f.objects), len(object_types))
        self.assertEqual([o.type for o in f.objects],
                         object_types)

        [
            polygon0,
            ellipse0,
            text0,
            ellipse1,
            text1,
            path1,
            polygon1
        ] = f.objects

        self.assertTrue(text0.get_text_raw() == "a" or
                        text1.get_text_raw() == "a")

        self.assertTrue(text0.get_text_raw() == "b"
                        or text1.get_text_raw() == "b")
