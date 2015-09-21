#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import unittest
import faint.extra.tesseract as tesseract
from py_tests import make_test_dir, test_data_dir
from faint import read_png
from faint.image import Pimage

class TestTesseract(unittest.TestCase):

    def test_tesseract(self):
        src = os.path.join(test_data_dir(), "tesseract-test.png")
        bmp, meta = read_png(src)
        img = Pimage(20, 20)
        img.background = bmp
        temp_dir = make_test_dir(self)

        ocr_text = tesseract.to_text(temp_dir, img).strip()
        self.assertEqual(ocr_text, "Hello world")
