#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import unittest

import faint.svg.parse_svg as svg
import faint
from py_ext_tests import test_data_dir, make_common_test_dir

def svg_file(file_name):
    return os.path.join(test_data_dir(), "svg", file_name + ".svg")

def png_key_file(file_name):
    return os.path.join(test_data_dir(), "svg", "key", file_name + ".png")

def target_png_name(root, file_name):
    return os.path.join(root, file_name + ".png")

def test_file(test, file_name):
    l = faint.PimageList()
    src_file_name = file_name
    svg.parse_doc(svg_file(src_file_name), l)
    out_dir = make_common_test_dir(test)
    img = l.frames[0]
    img.flatten()
    out_file = target_png_name(out_dir, src_file_name)
    key_path = png_key_file(file_name)

    if not os.path.exists(key_path):
        faint.write_png(img.background, out_file, faint.png.RGB)
        raise test.failureException("Key file missing: %s. Creating: %s" %
                                    (key_path, out_file))

    key, meta = faint.read_png(key_path)
    if not key == img.background:
        faint.write_png(img.background, out_file, faint.png.RGB)
        raise test.failureException("%s different to %s" % (out_file ,key_path))

class TestSVG(unittest.TestCase):
    # Fixme: Would be nicer if discovery was dynamic on svg:s in
    # test-data/svg/ folder, yet still separate test-functions.
    # Override test-discovery?

    def test_group_element(self):
        test_file(self, "group-element")

    def test_group_transform(self):
        test_file(self, "group-transform")

    def test_percentage(self):
        test_file(self, "percentage")

    def test_svg_element(self):
        test_file(self, "svg-element")

    def test_svg_element_viewbox(self):
        test_file(self, "svg-element-viewbox")

    def test_text_anchor(self):
        test_file(self, "text-anchor")
