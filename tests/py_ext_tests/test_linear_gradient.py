#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import LinearGradient

class TestLinearGradient(unittest.TestCase):

    def test_init_types(self):
        # Arguments required
        ar = self.assertRaises(ValueError)
        with ar: LinearGradient();
        with ar: LinearGradient(42.0);
        with ar: LinearGradient(0.0, (255,0,255));

        # Integer and float offset (was broken once!)
        LinearGradient((0, (255,0,255)))
        LinearGradient((0.0, (255,0,255)))
        LinearGradient((1, (255,0,255)))

        # Angle
        LinearGradient(42, (0.0, (255,0,255)))
        LinearGradient(42.0, (0.0, (255,0,255)))

        # Multiple color stops
        LinearGradient(42.0, (0.0, (255,0,255)), (1.0, (255,255,255)))


    def test_default_angle(self):
        lg1 =  LinearGradient((42, (255,0,255)))
        self.assertEqual(lg1.get_angle(), 0, "Default angle is 0")

        lg2 = LinearGradient(42, (0.0, (255, 0, 255)))
        self.assertAlmostEqual(lg2.get_angle(), 42.0)


    def test_color_stops(self):
        lg =  LinearGradient((0.0, (255,254,253)), (0.5, (0, 1, 2, 3)))

        stops = lg.get_stops()
        self.assertEqual(len(stops), 2)

        s0 = stops[0]
        self.assertEqual(s0[0], 0.0)
        self.assertEqual(s0[1], (255, 254, 253, 255))

        s1 = stops[1]
        self.assertAlmostEqual(s1[0], 0.5)
        self.assertEqual(s1[1], (0, 1, 2, 3))
