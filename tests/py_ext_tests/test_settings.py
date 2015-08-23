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

    def test_add(self):
        s = Settings()
        s.linewidth = 10
        self.assertEqual(s.linewidth, 10)
