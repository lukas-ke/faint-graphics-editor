#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2012 Lukas Kemmer
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You
# may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

from faint.pdf.xobject import XObject
from faint.pdf.stream import Stream

# Types of objects that should be included in the catalog
_catalogued = ["/Pages", "/Outlines"]

class pdf_id:
    """An object ID and a generation number (always 0 here) e.g. 1 0"""
    def __init__(self, num):
        self.num = num
    def __str__(self):
        return "%d 0" % self.num
    def reference(self):
        return "%d 0 R" % self.num
    def __hash__(self):
        return self.num
    def __lt__(self, other):
        return self.num < other.num
    def __eq__(self, other):
        return self.num == other.num

def _format_entry(key, value):
    """Outputs an entry in a PDF dictionary"""
    return "/" + key + " " + str(value)

class Object:
    """Base class for PDF objects with dictionaries"""
    def __init__(self):
        pass
    def __len__(self):
        return len(self.keys())
    def keys(self):
        return []
    def data(self):
        return ""

class Font(Object):
    """PDF Font object"""
    def __init__(self, baseFont):
        self.baseFont = baseFont
    def keys(self):
        return ["Type",
                "Subtype",
                "BaseFont"]
    def data(self):
        return ""
    def __getitem__(self, item):
        if item == "Type":
            return "/Font"
        elif item == "Subtype":
            return "/Type1"
        elif item == "BaseFont":
            return self.baseFont


class Pages(Object):
    """PDF Pages object, indicating the individual pages"""
    def __init__(self):
        self.kids = []
    def keys(self):
        return ["Type", "Kids", "Count"]
    def __len__(self):
        return len(self.keys())
    def __setitem__(self, item, value):
        pass
    def __getitem__(self, item):
        if item == "Count":
            return len(self.kids)
        elif item == "Kids":
            return "[" + " ".join([kid.reference() for kid in self.kids]) + "]"
        elif item == "Type":
            return "/Pages"
        else:
            assert(False)
    def add(self, id):
        self.kids.append(id)
    def data(self):
        return ""

class Resources(Object):
    def __init__(self):
        self.items = {}
        self.xobjects = []
    def __len__(self):
        return 0

    def add_proc_ref(self, procRef):
        self.items["ProcRef"] = procRef

    def add_xobject(self, object_id):
        self.xobjects.append(object_id)
        return self._xobject_index_to_name(len(self.xobjects) - 1)

    def __getitem__(self, item):
        return self.items[item]

    def __str__(self):
        proc_ref = self.items["ProcRef"].ref()
        rows = []
        rows.append(" /ProcSet %s R" % proc_ref)
        for xobj_num, xobj_ref in enumerate(self.xobjects):
            rows.append(" /XObject << %s %s R >>" % (
                    self._xobject_index_to_name(xobj_num),
                    xobj_ref))
        return "<< \n" + "\n".join(rows) + " >>"

    def _xobject_index_to_name(self, index):
        return "/X%d" % (index + 1)

class ProcRef:
    def __init__(self, doc):
        self.doc = doc
    def ref(self):
        return self.doc.get_resource_id()
    def __str__(self):
        return " << /ProcSet %d %d R >>" % self.doc.get_resource_id()

class Proc(Object):
    def data(self):
        return "[/PDF]\n"
    def __str__(self):
        return "[/PDF]\n"

class Document:
    """A PDF document composed of the various Objects and one or more pages
    containing a Stream each.

    """
    def __init__(self):
        self.identifiers = []
        self.objects = {}

        # The PDF /Catalog item
        self.catalog = {"Type" : "/Catalog"}
        self._append(self.catalog)

        # The PDF /Outlines item
        self.outlines = {"Type": "/Outlines", "Count" : 0}
        self._append(self.outlines)

        # The PDF /Pages item
        self.pages = Pages()
        self.pagesId = self._append(self.pages)
        self.resources = Resources()
        self.procSet = Proc()

        # Map of object numbers to lists of PDF comments. The comments
        # indicated by an id precede the object identified by that id
        # (as a way to know where the comments should go when
        # emitting)
        self.comments = {}

    def get_resource_id(self):
        # Fixme: Isn't this the ProcSet id?
        return pdf_id(len(self.identifiers) + 1)

    def get_xref_id(self):
        res_id = self.get_resource_id()
        return pdf_id(res_id.num + 1)

    def add_page(self, width, height):
        self.resources.add_proc_ref(ProcRef(self))
        page = {"Type" : "/Page",
                "MediaBox" : "[0 0 %d %d]" % (width, height),
                "Resources" : self.resources,
                "Parent" : self.pagesId.reference()
                }
        id = self._append(page)
        self.pages.add(id)
        return id

    def add_stream(self, stream, page_id):
        stream_id = self._append(stream)
        page = self.objects[page_id]
        page["Contents"] = stream_id.reference()

    def add_xobject(self, xobject):
        """Adds the specified xobject and returns the name"""
        obj_id = self._append(xobject)
        name = self.resources.add_xobject(obj_id)
        return name

    def add_comment(self, text):
        id = pdf_id(len(self.identifiers))
        if not id in self.comments:
            print("Appended.")
            self.comments[id] = []
        self.comments[id].append(text)

    def _append(self, obj):
        id = pdf_id(len(self.identifiers) + 1)
        self.identifiers.append(id)
        self.objects[id] = obj

        if "Type" in obj.keys():
            objType = obj["Type"]
            if objType in _catalogued:
                self.catalog[objType[1:]] = id.reference()
        return id

    def __str__(self):
        s = "%PDF-1.4\n"
        # Fixme: also add a row of non-printable characters
        index = []
        index.append(len(s)) # The Catalog-object follows the initial comment

        for key in self.identifiers:
            obj = self.objects[key]
            comments = self.comments.get(key, [])
            for comment in comments:
                s += "% " + comment + "\n"
            s = s + self._format_obj(key, obj)
            index.append(len(s))

        # Add the ProcSet before the xref index
        s = s + self._format_obj(self.get_resource_id(), self.procSet)
        index.append(len(s))
        startXref = len(s)
        s = s + self._format_xref(index)
        s = s + self._format_trailer(len(index))
        s = s + "startxref\n%d\n%%%%EOF\n" % startXref
        return s

    def _format_trailer(self, size):
        return "trailer\n<< /Size %d\n/Root 1 0 R\n>>\n" % size

    def _format_xref(self, index):
        """Returns a string for the cross-reference table."""
        return ("xref\n" +
                "0 %d\n" % (len(index)) + # Number of entries
                "0000000000 65535 f \n" + # Special entry
                " \n".join([str(offset).rjust(10,"0") + " 00000 n" for offset in index[:-1]]) + # Individual entries
                " \n") # Final end-line

    def _format_obj(self, key, obj):
        return ("%s obj\n" % str(key) +
                self._format_obj_dict(obj) +
                self._format_obj_data(obj)
                + "endobj\n")

    def _format_obj_dict(self, obj):
        if len(obj) == 0:
            return ""
        obj_dict = "<< " + "\n".join([_format_entry(key, obj[key])
                                      for key in obj.keys()])
        if len(obj.keys()) > 1 :
            obj_dict = obj_dict + "\n>>\n"
        else:
            obj_dict = obj_dict + " >>\n"
        return obj_dict

    def _format_obj_data(self, obj):
        if obj.__class__ == dict:
            return ""
        return obj.data()
