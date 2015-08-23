import faint
import unittest

class TestBitmap(unittest.TestCase):

    def test_construction(self):
        bmp = faint.Bitmap((123, 456  ))
        self.assertEqual(bmp.get_size(), (123,456))
        self.assertEqual(bmp.get_pixel(1,1), (255,255,255,255))

    def test_set_pixel(self):
        bmp = faint.Bitmap((10, 10))
        bmp.set_pixel((1,1),(255,0,255))
        self.assertEqual(bmp.get_pixel(1,1),(255,0,255,255))
