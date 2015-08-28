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

    def test_types(self):
        def check(obj, type_str):
            self.assertEqual(obj.type, type_str)

        r = (0, 0, 100, 100)
        pos = (20, 10)
        size = (10, 10)
        points = (0, 0, 20, 20, 10, 10, 30, 20)
        check(faint.create_Ellipse(r), "Ellipse")
        check(faint.create_Line(points), "Line")
        check(faint.create_Polygon(points), "Polygon")
        check(faint.create_Raster(pos, faint.Bitmap(size)), "Raster")
        check(faint.create_Raster(r, faint.Bitmap(size)), "Raster")
        check(faint.create_Rect(r), "Rectangle")
        check(faint.create_Spline(points), "Spline")
        check(faint.create_Text(pos, "Hello world"), "Text Region")
        # Fixme: Add Group

    def test_group(self):
        r = faint.create_Ellipse((0,0,100,100))
        e = faint.create_Rect((0,20,20,30))
        g = faint.create_Group(r, e)
        self.assertEqual(g.num_objs(), 2)
        self.assertEqual(g.get_obj(0).type, "Ellipse")
        self.assertEqual(g.get_obj(1).type, "Rectangle")

        with self.assertRaises(ValueError): # FIXME: IndexError
            g.get_obj(2)

        with self.assertRaises(ValueError): # FIXME: IndexError
            g.get_obj(-1)
