#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
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
