#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import unittest
from faint import ImageProps, Settings

class TestFrameProps(unittest.TestCase):

    def test_api(self):
        p = ImageProps()

        with self.assertRaises(IndexError):
            p.get_frame(0)

        f = p.add_frame(10,10)
        same = p.get_frame(0)
        # self.assertEqual(f0, same) # Fixme: Fails, FrameProps not equal to self

        with self.assertRaises(IndexError):
            f.Group(0)

        id_0 = f.Ellipse(0, 0, 100, 100, Settings())
        id_1 = f.Rect(0, 0, 100, 100, Settings())
        id_2 = f.Group(id_0, id_1)

        # Fixme error message is: "Flat string pare not starting at 0"
        # f.Text((0,0, 20, 30), Settings())

        f.Text((0, 0, 20, 30), "Hello", Settings())

        # TODO: Allow for unbounded text
        # f.Text((10, 20), "Hello", Settings())

        f.Line((0, 5, 10, 30), Settings())
        f.Path("m 10 10 l 20 20 30 10", Settings())

        with self.assertRaises(ValueError):
            # Should start with move
            f.Path("l 10 10 l 20 20 30 10", Settings())

        # Fixme: Not interpreted as an error?
        # with self.assertRaises(ValueError):
        #     f.Path("m 10 10 skwj10 10 l 20 20 30 10", Settings())

        f.add_warning("Oh man")

        # Fixme: Ugly extra parenthesis required.
        f.set_calibration(((0, 0, 20, 20), 10.0, "mm"))
