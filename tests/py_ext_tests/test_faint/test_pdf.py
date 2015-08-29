import unittest
import os
import faint
import faint.pdf.write_pdf as write_pdf
import py_ext_tests
from faint import settings

class TestWritePDF(unittest.TestCase):

    def test_write_pdf(self):
        l = faint.PimageList()
        f = l.add_frame((120, 120))

        f.Rect((10,10,100,100),
               settings(fg=(0,0,255),
                             bg=(128,128,255),
                             fillstyle='bf',
                             linewidth=1))
        f.Ellipse((10, 10, 100, 100), settings(fg=(255,0,0),linestyle='ld'))
        f.Line((10, 10, 109, 109))
        f.Line((10, 109, 109, 10)) # Fixme: 110 cuts the rectangle
        f.Text((10, 10, 100, 100), "Hello")

        out_dir = py_ext_tests.make_test_dir(self)
        s = write_pdf.write(os.path.join(out_dir, "test.pdf"), l)
