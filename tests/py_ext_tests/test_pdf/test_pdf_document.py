#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import unittest

from faint.pdf.xobject import XObject
from faint.pdf.document import Document
from faint.pdf.stream import Stream

from py_ext_tests import make_common_test_dir

class TestPdfDocument(unittest.TestCase):

    def _write_pdf(self, doc, name):
        out_dir = make_common_test_dir(self)
        target_file = os.path.join(out_dir, name)
        with open(target_file, 'w', newline='\n') as f:
            f.write(str(doc))

    def test_multipage(self):
        s = Stream()
        doc = Document()
        s.text(x=0, y=400, size=24, string="Page 1")

        page_id1 = doc.add_page(640, 480)
        doc.add_stream(s, page_id1)

        page_id2 = doc.add_page(640, 400)
        s = Stream()
        s.text(x=0, y=300, size=24, string="Page 2")
        doc.add_stream(s, page_id2)

        page_id3 = doc.add_page(400, 640)
        s = Stream()
        s.text(x=0, y=500, size=24, string="Page 3")
        doc.add_stream(s, page_id3)
        self._write_pdf(doc, "test_multipage.pdf")

    def test_stream(self):
        s = Stream()
        doc = Document()
        s.fgcol(0.0, 0.0, 0.0)
        s.line(0, 0, 100, 100)
        s.ellipse(0, 0, 140, 100)
        s.stroke()
        s.ellipse(100, 100, 80, 40)
        s.stroke()
        s.ellipse(160, 160, 80, 80)
        s.stroke()
        s.text(x=0, y=400, size=12, string="Hello")

        page_id1 = doc.add_page(640, 480)
        doc.add_stream(s, page_id1)

        self._write_pdf(doc, "test_stream.pdf")

    def test_xobject(self):
        def _make_test_xobject():
            return XObject(
                "\xff\x00\x00\x00\xff\x00\x00\x00\xff"
                "\xff\x00\xff\x00\x00\x00\xff\x00\xff"
                "\xff\x00\x00\x00\xff\x00\x00\x00\xff",
                width=3, height=3)

        doc = Document()
        page_id1 = doc.add_page(640, 480)
        xobject_name = doc.add_xobject(_make_test_xobject())

        s = Stream()
        s.fgcol(0.0, 0.0, 0.0)
        s.xobject(xobject_name, 0, 0, 640, 480)
        doc.add_stream(s, page_id1)
        self._write_pdf(doc, "test_xobject.pdf")
