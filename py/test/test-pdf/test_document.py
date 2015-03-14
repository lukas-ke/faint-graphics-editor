#!/usr/bin/env python3
from faint.pdf.xobject import XObject
from faint.pdf.document import Document
from faint.pdf.stream import Stream

def _make_test_xobject():
    return XObject(
        "\xff\x00\x00\x00\xff\x00\x00\x00\xff"
        "\xff\x00\xff\x00\x00\x00\xff\x00\xff"
        "\xff\x00\x00\x00\xff\x00\x00\x00\xff",
        width=3, height=3)

def test_multipage():
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

    with open("out-multipage.pdf", 'w', newline='\n') as f:
        f.write(str(doc))

def test_stream():
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

    with open("out-stream.pdf", 'w', newline='\n') as f:
        f.write(str(doc))


def test_xobject():
    doc = Document()
    page_id1 = doc.add_page(640, 480)
    xobject_name = doc.add_xobject(_make_test_xobject())

    s = Stream()
    s.fgcol(0.0, 0.0, 0.0)
    s.xobject(xobject_name, 0, 0, 640, 480)
    doc.add_stream(s, page_id1)
    with open("out-xobject.pdf", 'w', newline='\n') as f:
        f.write(str(doc))

if __name__ == '__main__':
    test_stream()
    test_xobject()
    test_multipage()
