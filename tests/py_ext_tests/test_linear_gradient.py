#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import LinearGradient

class TestLinearGradient(unittest.TestCase):

    def test_init(self):
        lg = LinearGradient((0, (255,0,255)))
        lg = LinearGradient((0.0, (255,0,255)))
