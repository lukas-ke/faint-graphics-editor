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


    def test_angle(self):
        lg1 =  LinearGradient((42, (255,0,255)))
        # Fixme: Replace set, get_angle with property
        self.assertEqual(lg1.get_angle(), 0, "Default angle is 0")

        lg2 = LinearGradient(42, (0.0, (255, 0, 255)))
        self.assertAlmostEqual(lg2.get_angle(), 42.0)

        lg1.set_angle(20.0)
        self.assertEqual(lg1.get_angle(), 20.0)


    def test_color_stops(self):
        lg =  LinearGradient((0.0, (255,254,253)), (0.5, (0, 1, 2, 3)))
        self.assertEqual(lg.get_num_stops(), 2)
        s0, s1 = lg.get_stops()

        self.assertEqual(s0, (0.0, (255, 254, 253, 255)))
        self.assertEqual(s1, (0.5, (0, 1, 2, 3)))

        lg.add_stop(0.6, (50, 60, 70))
        stops = lg.get_stops()
        self.assertEqual(len(stops), 3)

        s2 = stops[2]
        self.assertEqual(s2, (0.6, (50, 60, 70, 255)))
        self.assertEqual(s2, lg.get_stop(2))

        with self.assertRaises(IndexError):
            lg.get_stop(3)

        with self.assertRaises(ValueError):
            lg.get_stop(-1)
