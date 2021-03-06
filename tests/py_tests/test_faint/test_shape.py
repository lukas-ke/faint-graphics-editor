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
        ce = faint.create_Ellipse
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
        check(faint.create_Group(ce(r), ce(r)), "Group")


    def test_group(self):
        e = faint.create_Ellipse((0,0,100,100))
        r = faint.create_Rect((0,20,20,30))
        g = faint.create_Group(e, r)
        self.assertEqual(g.num_objs(), 2)
        self.assertEqual(g.get_obj(0).type, "Ellipse")
        self.assertEqual(g.get_obj(1).type, "Rectangle")

        self.assertRaises(IndexError, g.get_obj, 2)
        self.assertRaises(IndexError, g.get_obj, -1)

        r2 = faint.create_Rect(100, 120, 220, 200)
        r3 = faint.create_Rect(100, 120, 220, 200)
        g2 = faint.create_Group(r2, r3, g)
        self.assertEqual(g2.num_objs(), 3)
        self.assertEqual(g2.get_obj(2).num_objs(), 2)
