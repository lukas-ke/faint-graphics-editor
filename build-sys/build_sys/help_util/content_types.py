#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2014 Lukas Kemmer
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

import codecs
import os
import re

def html_link(url, label=None):
    if label is None:
        label = url
    return '<a href="%s">%s</a>' % (url, label)

def check_table_style(table_style):
    if not table_style in ("fancy", "plain"):
        raise ContentError("Invalid tablestyle: %s" % table_style)

def _get_source_image_path(filename):
    return os.path.join("../help/images", filename)

def _get_source_graphic_path(filename):
    return os.path.join("../graphics/", filename)

def html_help_image(filename):
    return '<img src="images/%s"></img>' % filename

def html_image_map(state, map_file_name, image_file_name):
    map_name = state.new_map_name()
    lines = codecs.open(_get_source_image_path(map_file_name)).read().decode("ascii")
    if lines.find('\r') != -1:
        raise ContentError("Non-unix endline (\\r) in %s" % map_file_name)

    res = '<img src="images/%s" usemap="#%s"></img>' % (image_file_name, map_name)
    res += '<map name="%s">' % map_name
    for line in lines.split('\n'):
        if len(line) != 0:
            coords, target = line.split(' ')
            target_label = state.labels.get(target)
            if target_label is None:
                raise(ContentError("Undefined label: %s in image map %s" % (target, map_file_name)))
            res += '<area SHAPE="rect" COORDS="%s" HREF="%s">' % (coords, target_label.url)
    res += '</map>'
    return res

def _tbl_str_width(num, widths):
    """Returns the width-argument for an html table cell using the width from
    the list of widths.
    Returns width="[some number]" or the empty-string if no width is
    specified for this num"""
    if num >= len(widths):
        return ""
    return ' width="%s"' % widths[num]


class ContentError(Exception):
    def __init__(self, error):
        Exception.__init__(self, error)


class Bullet:
    def __init__(self, items):
        self.items = items

    def to_html(self, state, **kwArgs):
        return '<li>%s</li>' % "".join([item.to_html(state) for
                                        item in self.items])

    def __repr__(self):
        return "Bullet()"


class ConfigPath:
    def to_html(self, state, **kwArgs):
        return "<FAINTCONFIGPATH>"


class ChildSummary:
    def __init__(self):
        pass
    def to_html(self, state, **kwArgs):
        return ""
    def __repr__(self):
        return "ChildSummary"


class ExternalReference:
    def __init__(self, url):
        if not url.startswith("http://") and not url.startswith("https://"):
            raise ContentError('External url not prefixed by http://. "%s"' % url)
        self.url = url

    def to_html(self, state, **kwArgs):
        return html_help_image("icon-extref.png") + html_link(self.url)


class Footer:
    def __init__(self, targetname, link):
        self.targetname = targetname
        self.link = link
        self.title = ""

    def set_title(self, title):
        self.title = title

    def to_html(self, state, **kwArgs):
        return '<hr><a href="%s">Next</a> - %s' % (self.link, self.title)


class Graphic:
    def __init__(self, filename):
        self.filename = filename
        relative_path = _get_source_graphic_path(filename)
        if not os.path.exists(relative_path):
            raise ContentError("Missing graphic: %s" % relative_path)
    def to_html(self, state, **kwArgs):
        return '<img src="../graphics/%s"></img>' % self.filename
    def __repr__(self):
        return "Graphic(%s)" % self.filename


class Header:
    def __init__(self, targetname, link):
        self.targetname = targetname
        self.link = link
        self.title = ""

    def set_title(self, title):
        self.title = title

    def to_html(self, state, **kwArgs):
        return '<a href="%s">Previous</a> - %s<hr>' % (self.link, self.title)


class HR:
    def to_html(self, state, **kwArgs):
        return '<hr>'


class Image:
    _referenced_images = ["non-versioned",
                          "icon-extref.png",
                          "key-alt.png", # Fixme
                          "key-delete.png", # Fixme
                          ".svn",
                         ]
    def __init__(self, filename):
        self.filename = filename
        Image._referenced_images.append(filename)
        relative_path = _get_source_image_path(filename)
        if not os.path.exists(relative_path):
            raise ContentError("Missing image: %s" % relative_path)
    def to_html(self, state, **kwArgs):
        return html_help_image(self.filename)
    def __repr__(self):
        return "Image(%s)" % self.filename


class ImageMap:
    def __init__(self, filename):
        self.map_file_name = filename
        assert(self.map_file_name.endswith(".map"))
        self.image_file_name = filename[:-4] + ".png"
        Image._referenced_images.append(self.image_file_name)
        Image._referenced_images.append(self.map_file_name)
        imgRelPath = _get_source_image_path(self.image_file_name)
        if not os.path.exists(imgRelPath):
            raise ContentError("Missing image for image map: %s" % imgRelPath)
        imgMapRelPath = _get_source_image_path(self.map_file_name)
        if not os.path.exists(imgMapRelPath):
            raise ContentError("Missing image map: %s" % imgMapRelPath)

    def to_html(self, state, **kwArgs):
        return html_image_map(state, self.map_file_name, self.image_file_name)

    def __repr__(self):
        return "ImageMap(%s)" % self.map_file_name


class Include:
    def __init__(self, filename):
        include_base = '../help/source'
        self.filename = os.path.join(include_base, filename)

        if not os.path.exists(self.filename):
            raise ContentError("Missing include: %s" % self.filename)
    def to_html(self, state, **kwArgs):
        with open(self.filename) as f:
            return f.read()
    def __repr__(self):
        return "Include(%s)" % self.filename


class Instruction:
    """A configuration item for the output"""
    def __init__(self, name, values):
        self.name = name
        self.values = values
    def to_html(self, state, **kwArgs):
        return ""

    def update_state(self, state):
        if self.name == "tablewidths":
            state.table_widths = self.values


class Label:
    def __init__(self, label, title, url):
        self.label = label
        self.title = title
        self.url = url

    def to_html(self, state, **kwArgs):
        return '<a name="%s"></a>' % self.label

    def __repr__(self):
        return "Label(%s)" % self.label


class Page:
    def __init__(self, filename, sourcename, title, summary, items):
        self.items = items
        self.title = title
        self.summary = summary
        self.filename = filename
        self.sourcename = sourcename

    def first_title(self):
        for item in self.items:
            if item.__class__ == Title:
                return item.text
        return ""


class PythonBind:
    """A Python function bind, rendering as the key-code"""
    def __init__(self, funcName):
        self.funcName = funcName

    def to_html(self, state, **kwArgs):
        return "<bind>%s</bind>" % self.funcName


class PythonBindList:
    def to_html(self, state, **kwArgs):
        return "<bindlist></bindlist>"
    def __repr__(self):
        return "PythonBindList()"


class PythonFunction:
    def __init__(self, signature, withBind=True):
        parenPos = signature.find("(")
        assert(parenPos != -1)
        self.name = signature[:parenPos]
        self.args = signature[parenPos:]
        self.withBind = withBind
    def to_html(self, state, **kwArgs):
        if self.withBind:
            return self._name_and_args() + " " + "<bind>%s</bind>" % self.name
        return self._name_and_args()

    def _name_and_args(self):
        return self.name + self.args


class PythonSample:
    """Insert and format Python code sample"""

    def __init__(self, filename):
        self.filename = filename

    def _py_line(self, l):
        return l.replace(" ", "&nbsp;").replace("<", "&lt;").replace(">", "&gt;").replace("##", "")

    def to_html(self, state, **kwArgs):
        text = '<font face="courier">'
        f = open('../help/example_py/%s' % self.filename)
        lines = [l.rstrip() for l in f.readlines()]
        try:
            lines = lines[lines.index("#start") + 1:]
        except ValueError:
            raise ContentError('PythonSample %s missing #start-comment' % self.filename)
        text += '<br>'.join([self._py_line(l) for l in lines])
        text += '</font>'
        return text


class Reference:
    def __init__(self, state, name, label=None):
        self.name = name
        self.target_label = None
        self.custom_label = label
        self.state = state

    def to_html(self, state, **kwArgs):
        if self.target_label is None:
            self.target_label = self.state.labels.get(self.name, None)
        if self.target_label is None:
            raise(ContentError("Undefined label: %s" % self.name))
        if self.custom_label is not None:
            title = self.custom_label
        else:
            title = self.target_label.title
        return '<a href="%s">%s</a>' % (self.target_label.url, title)


class Paragraph:
    def to_html(self, state, **kwArgs):
        return "<p>"


class Text:
    html_replacements = [("\\br", "<br>"),
                         ("->", "&rarr;"), # Ascii arrow as html-right arrow
                         (u"é", "&eacute;")]
    def __init__(self, text):
        self.text = text
    def to_html(self, state, **kwArgs):
        html_str = self.text
        for src, replacement in self.html_replacements:
            html_str = html_str.replace(src, replacement)

        return html_str
    def __repr__(self):
        return "Text(...)"


class Table:
    def __init__(self, line, content_parser):
        self.cells = line.split("||")[1:-1]
        self.content_parser = content_parser

    def to_html(self, state, **kwArgs):
        fmt_cells = ["".join([item2.to_html(state) for item2 in item])
                     for item in [self.content_parser(state, item)
                                  for item in self.cells]]
        row = kwArgs["row"]
        table_style = state.table_style
        table_widths = state.table_widths
        check_table_style(table_style)
        widths = []
        for num, cell in enumerate(fmt_cells):
            if num < len(table_widths):
                widths.append(table_widths[num])

        html = "<tr>"
        for num, cell in enumerate(fmt_cells):
            if table_style == "fancy" and row == 0:
                html = html + '<td bgcolor="#F5E49C"%s>%s</td>' % (_tbl_str_width(num, widths), cell)
            elif row % 2 == 0 and table_style == "fancy" and row != 0:
                html = html + '<td bgcolor="%s"%s>%s</td>' % (state.color_table_odd, _tbl_str_width(num, widths), cell)
            else:
                html = html + '<td%s>%s</td>' % (_tbl_str_width(num,widths), cell)
        html = html + "</tr>"
        return html

class TableStyle:
    def __init__(self, style):
        self.style = style
        check_table_style(style)
    def to_html(self, state, **kwArgs):
        return ""

    def update_state(self, state):
        state.table_style = self.style


class Tagged:
    def __init__(self, tag, content, content_parser):
        self.tag = tag
        self.content = content
        self.content_parser = content_parser

    def to_html(self, state, **kwArgs):
        html = "<%s>%s</%s>" % (self.tag, "".join(item.to_html(state) for item in self.content_parser(state, self.content)), self.tag)
        return html


class Title:
    def __init__(self, text, level):
        self.text = text
        self.level = level
    def to_html(self, state, **kwArgs):
        return "<h%d>%s</h%d>" % (self.level, self.text, self.level)
    def __repr__(self):
        return "Title"

def escape_space(match):
    return "&nbsp;" * len(match.group(0))

class VerbatimInclude:
    def __init__(self, path):
        include_base = '../help/source'
        self.path = os.path.join(include_base, path)
        if not os.path.exists(self.path):
            raise ContentError("Missing verbatim_include: %s" % self.path)

    def to_html(self, state, **kwArgs):
        with open(self.path) as f:
            text = f.read()
            if not self.path.endswith(".html"):
                text = re.sub("^([ ]+)", escape_space, text, flags=re.MULTILINE)
                text = text.replace("\n", "<br>")

        return ('<table border="1" width="100%"><tr>' +
                '<td bgcolor="#ffffff" width="100%">' +
                ('<font face="courier new">'
                 if not self.path.endswith(".html") else "") +
                text +
                ('</font>' if not self.path.endswith(".html") else "") +
                '</td></tr></table>')

    def __repr__(self):
        return "VerbatimInclude"
