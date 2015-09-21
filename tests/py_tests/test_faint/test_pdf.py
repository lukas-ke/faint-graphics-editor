import unittest
import os
import faint
import faint.pdf.write_pdf as write_pdf
import faint.pdf.parse_pdf as parse_pdf
import py_tests
from faint import settings

class TestWritePDF(unittest.TestCase):

    def test_pdf(self):
        # Write a PDF
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

        out_dir = py_tests.make_test_dir(self)
        filename = os.path.join(out_dir, "test.pdf")
        write_pdf.write(filename, l)


        # Read it back
        l2 = faint.PimageList()
        parse_pdf.parse(filename, l2)
        self.assertEqual(len(l2.frames), 1)

        f2 = l2.frames[0]
        self.assertEqual(len(f2.objects), 5)

        object_types = ['Polygon', # Fixme: Was Rect
                        "Ellipse",
                        "Line",
                        "Line",
                        "Text Region"]
        self.assertEqual([o.type for o in f2.objects],
                         object_types)

        self.assertEqual(f2.objects[-1].get_text_raw(), "Hello")
