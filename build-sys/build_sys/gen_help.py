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

import os
import sys
import codecs
from build_sys.help_util.markup_regex import *
from build_sys.help_util.content_types import *
from os.path import join

HELP_OUT_FOLDER = "../help"
HELP_SOURCE_FOLDER = "../help/source"

def fail_with_message(description):
    print("Error: " + description)
    sys.exit(1)

class ParseState:
    """The state of the parsing, e.g. found labels and what not."""

    # Fixme: Is kinda both ParseState and WriteState.
    def __init__(self):

        # Labels found in the parsed files, for cross-referencing
        self.labels = {}

        # The last specified table style
        self.table_style = "plain"

        # The last table width specification
        self.table_widths = []

        # Odd or even row
        self.table_odd = False

        # Page background color
        self.color_background = "#FAFDD5"

        # Backgrounds for even and odd table rows
        self.color_table_even = self.color_background
        self.color_table_odd = "#F9FCC4"
        self.color_link="#4D6DF3"
        self.map_num = 0


    def new_page(self):
        self.table_style = 'plain'
        self.table_widths = []


    def new_map_name(self):
        """Generates a new html image map identifier."""
        self.map_num += 1
        return "map%d" % self.map_num


def match_title(state, line):
    """Returns a match object if the line is matched by any title regex,
    otherwise None.

    """
    for num, title in enumerate([title1, title2, title3]):
        match = title.match(line)
        if match:
            return "".join([item.to_html(state) for item in parse_rest(state,
                                                                  match.group(1))]), num + 1
    return None


def list_split(regex, data, marker):
    ret = []
    for item in data:
        if item.startswith("["):
            ret.append(item)
            continue
        for num, splat in enumerate(regex.split(item)):
            if (num + 1) % 2 == 0:
                ret.append(marker + splat)
            elif len(splat) > 0:
                ret.append(splat)
    return ret


def to_object(state, txt):
    if txt.startswith("[R]"):
        content = txt[3:]
        content = content.split(',')
        if len(content) == 1:
            return Reference(state, content[0])
        else:
            return Reference(state, content[0], content[1])

    if txt.startswith("[IMGMAP]"):
        return ImageMap(txt[8:])
    if txt.startswith("[IMG]"):
        return Image(txt[5:])
    if txt.startswith("[GRAPHIC]"):
        return Graphic(txt[9:])
    if txt.startswith("[I]"):
        return Tagged('i', txt[3:], parse_rest)
    if txt.startswith("[B]"):
        return Tagged('b', txt[3:], parse_rest)
    if txt.startswith("[C]"):
        return Tagged('center', txt[3:], parse_rest)
    if txt.startswith("[WBND]"):
        return PythonFunction(txt[6:], True)
    if txt.startswith("[BINDLIST]"):
        return PythonBindList()
    if txt.startswith("[CONFIGPATH]"):
        return ConfigPath()
    if txt.startswith("[BIND]"):
        return PythonBind(txt[6:])
    if txt.startswith("[EXTREF]"):
        return ExternalReference(txt[8:])
    if txt.startswith("[PY_SAMPLE]"):
        return PythonSample(txt[11:])
    return Text(txt)


def parse_rest(state, rest):
    if rest.__class__ != [].__class__:
        rest = [rest]
    rest = list_split(re_verbatim_include, rest, "[VERBINCLUDE]")
    rest = list_split(re_ref, rest, "[R]")
    rest = list_split(re_image, rest, "[IMG]")
    rest = list_split(re_image_map, rest, "[IMGMAP]")
    rest = list_split(re_include, rest, "[INCLUDE]")
    rest = list_split(re_graphic, rest, "[GRAPHIC]")
    rest = list_split(re_withbind, rest, "[WBND]")
    rest = list_split(re_bind, rest, "[BIND]")
    rest = list_split(re_extref, rest, "[EXTREF]")
    rest = list_split(re_py_sample, rest, "[PY_SAMPLE]")
    rest = list_split(re_bindlist, rest, "[BINDLIST]")
    rest = list_split(re_configpath, rest, "[CONFIGPATH]")
    rest = list_split(italic, rest, "[I]")
    rest = list_split(bold, rest, "[B]")
    rest = list_split(center, rest, "[C]")
    return [to_object(state, item) for item in rest]


def parse_table_widths(s):
    """Returns a list of the widths in the string
    for group(1) of a  matched table_widths regex"""
    return [item.strip() for item in s.split(",")]


def read_file(filename):
    with codecs.open(filename, 'r', 'utf-8') as src:
        return src.read()


def insert_page(insert_match):
    include_base = '../help/source'
    filename = join(include_base, insert_match.group(1))
    return read_file(filename)


def source_file_name_to_target(file_path):
    file_name = os.path.split(file_path)[-1]

    # Fixme: Don't determine output format already
    return file_name.replace(".txt", ".html")


def parse_page_title(line):
    return line.split(":")[1].strip()[1:-1]


def parse_file(filename, prev, next, state):
    text = read_file(filename)

    # Replace includes references with the raw content from the other page.
    text = re_insert.sub(insert_page, text)

    # Strip comments
    text = re_comment.sub("", text)

    lines = [l + "\n" for l in text.split('\n')]

    if not lines[0].startswith("page:"):
        raise ContentError('First line does not start with "page:"')

    for line in lines:
        if line.find("\r") != -1:
            raise ContentError('Non-unix endline (\\r) found')

    page_title = parse_page_title(lines[0])

    target_filename = source_file_name_to_target(filename)
    summary = ""

    doc = [] # The list of items in the page
    if prev is not None:
        doc.append(Header(prev, prev.replace(".txt", ".html")))  # Fixme: Don't determine output format already
    in_list = False

    verbatim = False
    last_title = None

    for line in lines[1:]:
        if verbatim:
            if line.strip() == "}}}":
                verbatim = False
            else:
                doc.append(Text(line))
            continue

        if line.strip() == "{{{":
            verbatim = True
            continue

        m = re_summary.match(line)
        if m is not None:
            summary = m.group(1)
            continue

        m = re_summarize_children.match(line)
        if m is not None:
            doc.append(ChildSummary())
            continue

        m = hr.match(line)
        if m is not None:
            doc.append(HR())
            continue
        m = match_title(state, line)
        if m is not None:
            last_title = Title(m[0], m[1])
            doc.append(last_title)
            continue
        m = bullet.match(line)
        if m is not None:
            doc.append(Bullet(parse_rest(state, m.group(1))))
            continue

        m = label.match(line)
        if m is not None:
            name = m.group(1).strip()
            if last_title is not None:
                labelTitle = last_title.text
            else:
                fail_with_message("No title for " + filename)
            lbl = Label(name, labelTitle, target_filename + "#" + name )
            doc.append(lbl)
            state.labels[name] = lbl
            continue

        m = table_style.match(line)
        if m is not None:
            doc.append(TableStyle(m.group(1)))
            continue

        m = table_widths.match(line)
        if m is not None:
            doc.append(Instruction("tablewidths", parse_table_widths(m.group(1))))
            continue

        if line.strip() == "":
            doc.append(Paragraph())
            continue

        m = table_row.match(line)
        if m is not None:
            doc.append(Table(line, parse_rest))
            continue

        content = [line]
        content = list_split(re_verbatim_include, content,"[VERBINCLUDE]")
        content = list_split(re_extref, content, "[EXTREF]")
        content = list_split(re_py_sample, content, "[PY_SAMPLE]")
        content = list_split(center, content, "[C]")
        content = list_split(re_image, content,"[IMG]")
        content = list_split(re_image_map, content,"[IMGMAP]")
        content = list_split(re_withbind, content, "[WBND]")
        content = list_split(re_bind, content, "[BIND]")
        content = list_split(re_bindlist, content, "[BINDLIST]")
        content = list_split(re_configpath, content, "[CONFIGPATH]")
        content = list_split(re_include, content,"[INCLUDE]")
        content = list_split(re_graphic, content,"[GRAPHIC]")
        content = list_split(bold, content, "[B]")
        content = list_split(italic, content, "[I]")
        content = list_split(re_ref, content, "[R]")

        for item in content:
            if item.startswith("[IMG]"):
                doc.append(Image(item[5:]))
            elif item.startswith("[INCLUDE]"):
                doc.append(Include(item[9:]))
            elif item.startswith("[GRAPHIC]"):
                doc.append(Graphic(item[9:]))
            elif item.startswith("[B]"):
                doc.append(Tagged("b", item[3:], parse_rest))
            elif item.startswith("[U]"):
                doc.append(Tagged("u", item[3:], parse_rest))
            elif item.startswith("[I]"):
                doc.append(Tagged("i", item[3:], parse_rest))
            elif item.startswith("[C]"):
                doc.append(Tagged("center", item[3:], parse_rest))
            elif item.startswith("[VERBINCLUDE]"):
                doc.append(VerbatimInclude(item[13:]))
            else:
                doc.append(to_object(state, item))

    if next is not None:
        doc.append(Footer(next, next.replace(".txt", ".html")))
    else:
        doc.append(HR())
    return Page(target_filename, os.path.split(filename)[-1], page_title, summary, doc)


def write_child_summary(out, pages, child_pages):
    out.write('<table border="0" cellpadding="5">')
    for child in child_pages:
        out.write('<tr><td><a href="%s">%s</a></td><td width="10"></td><td>%s</td></tr>' % (pages[child].filename, pages[child].title, pages[child].summary))
    out.write('</table>')


def page_preamble(state, page):
    return (('<html><head><title>%s</title></head>' +
            '<body bgcolor="%s" leftmargin="50" link="%s">')
            % (page.title, state.color_background, state.color_link))

def write(out_dir, pages, page_hierarchy, state):
    row_num = 0
    for sourcename in pages:
        state.new_page()
        page = pages[sourcename]

        outfile = open(join(out_dir, page.filename), 'w')
        outfile.write(page_preamble(state, page))
        in_table = False
        in_list = False
        for item in page.items:
            if 'update_state' in item.__class__.__dict__: # Fixme: Remove
                item.update_state(state)

            if item.__class__ == Table:
                # Fixme: Move into Table-class
                # ... or add some kind of statefulness for Table and Bullet
                if not in_table:
                    row_num = 0
                    if state.table_style == "fancy":
                        outfile.write('<table border="0" cellpadding="5" width="80%">')
                    else:
                        outfile.write('<table border="0" cellpadding="5">')

                    in_table = True
                    state.table_odd = not state.table_odd

                else:
                    row_num += 1
            elif in_table:
                outfile.write('</table>')
                row_num = 0
                in_table = False

            if item.__class__ == Bullet:
                # Fixme: Move into class
                if not in_list:
                    in_list = True
                    outfile.write("<ul>\n")
            elif in_list:
                outfile.write("</ul>\n")
                in_list = False

            if item.__class__ == ChildSummary:
                write_child_summary(outfile, pages, page_hierarchy[sourcename])
            try:
                outfile.write(item.to_html(state,row=row_num))
            except ContentError as e:
                fail_with_message(str(e) + " (%s)" % page.sourcename)
        outfile.write("</body></html>")


def _need_generate(srcRoot, dstRoot, sources):
    """True if any out-file is missing or if a source file is modified
    more recently than its out file

    """

    dst_contents = join(dstRoot, "contents.dat")
    src_contents = join(srcRoot, "contents.txt")
    if not os.path.isfile(dst_contents) or os.path.getmtime(dst_contents) < os.path.getmtime(src_contents):
        return True

    for sourceFile, page in [(join(srcRoot, page), page) for page in sources]:
        targetFile = join(dstRoot, page.replace(".txt", ".html"))
        if not os.path.isfile(targetFile):
            # Html-target file missing for this source file
            return True
        if os.path.getmtime(targetFile) < os.path.getmtime(sourceFile):
            # An html page is outdated compared to its source text
            return True
    return False


def get_files_from_contents(content_lines):

    """Returns the help source file names with depth prefixes stripped."""
    lines = [line.strip() for line in content_lines]
    return [line.replace(">", "") for line in lines if not len(line) == 0]


def read_contents_source(contents_path):
    """Reads all source file name entries (including ">"-depth-prefixes) from
    the help-contents file."""
    f = open(contents_path)
    lines = [line.strip() for line in f.readlines()]
    return [line for line in lines if not len(line) == 0]


def write_contents(contentLines, dst_path, pages):
    contents_file = open(dst_path, 'w')
    for line in contentLines:
        if line.startswith(">"):
            contents_file.write(">")
        pageInfo = pages[line.replace(">","")]
        contents_file.write("%s;%s\n" % (pageInfo.title, pageInfo.filename))


def parse_contents(contentLines):
    pages = {}
    mainPage = ""
    for line in contentLines:
        line = line.strip()
        if not line.startswith(">"):
            mainPage = line
            pages[line] = []
        else:
            pages[line[1:]] = []
            pages[mainPage].append(line[1:])
    return pages


def find_unused_pages(referenced_pages, source_root_folder):
    """Returns any source help files in source_root_folder missing from
    the referenced_pages list.

    """
    all_pages = [page for page in os.listdir(source_root_folder) if
                page.endswith(".txt") and
                page != "contents.txt"]
    return [page for page in all_pages if page not in referenced_pages]


def find_unused_images(imageRoot, referenced_images):
    all_images = [image for image in os.listdir(imageRoot)]
    return [image for image in all_images if image not in referenced_images]


def init_headers_and_footers(pages):
    """Sets the text for the previous- and next- links in the header and
    footer on each page to the first section title on the
    preceeding/succeeding pages.

    """
    for source_page in pages:
        footer = pages[source_page].items[-1]
        if footer.__class__ == Footer:
            footer.set_title(pages[footer.targetname].first_title())

        header = pages[source_page].items[0]
        if header.__class__ == Header:
            header.set_title(pages[header.targetname].first_title())


def run(force=False):
    """Generates the Faint html-help if any help-source file has changed
    or force is True.

    """
    state = ParseState()

    # Retrieve what files to parse as pages from the contents file
    content = read_contents_source(join(HELP_SOURCE_FOLDER, "contents.txt"))
    source_pages = get_files_from_contents(content)
    unused_pages = find_unused_pages(source_pages, HELP_SOURCE_FOLDER)
    if len(unused_pages) != 0:
        fail_with_message("Unreferenced pages: %s" % ", ".join(unused_pages))

    pageHierarchy = parse_contents(content)
    if not force and not _need_generate(HELP_SOURCE_FOLDER, HELP_OUT_FOLDER, source_pages):
        print("* Html-help up to date.")
        return
    else:
        print("* Generating Html-help")

    pages = {}
    for num, page in enumerate(source_pages):
        if num == 0:
            # No previous page for the first page
            prev = None
        else:
            prev = source_pages[num - 1]
        if num == len(source_pages) -1:
            # No next page for the last page
            next = None
        else:
            next = source_pages[num + 1]

        try:
            current_file = join(HELP_SOURCE_FOLDER, page)
            pages[page] = parse_file(current_file, prev, next, state)
        except ContentError as e:
            fail_with_message(str(e) + " (%s)" % page)

    for source_name in source_pages:
        # Add page file-names as labels for \ref
        page = pages[source_name]
        state.labels[source_name] = Label(source_name, page.title, page.filename)

    init_headers_and_footers(pages)

    # Write all pages
    write(HELP_OUT_FOLDER, pages, pageHierarchy, state)

    # Write the contents (for the tree view)
    write_contents(content, join(HELP_OUT_FOLDER, "contents.dat"), pages)

    unused_images = find_unused_images(join(HELP_OUT_FOLDER, "images"),
                                       Image._referenced_images)
    if len(unused_images) != 0:
        fail_with_message("Unused images: %s" % ", ".join(unused_images))
