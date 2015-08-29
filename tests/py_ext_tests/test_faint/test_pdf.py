import unittest
import os
import faint
import faint.pdf.write_pdf as write_pdf
import py_ext_tests

class TestWritePDF(unittest.TestCase):

    def test_write_pdf(self):
        l = faint.PimageList()
        f = l.add_frame((640, 480))
        f.Rect((0,0,100,100))

        out_dir = py_ext_tests.make_test_dir(self)
        s = write_pdf.write(os.path.join(out_dir, "test.pdf"), l)
