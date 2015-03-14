#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2013 Lukas Kemmer
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

"""Parses a given cpp-file and finds functions prefixed with
  >  /* method "[doc-string]" */
and adds them to to a PyMethodDef list saved to a new file.

"""
import re
import os

import build_sys.util.util as util


class regex:
    """Regular expressions to match special C++-comments before methods
    (for methoddefs) and structs (for properties)

    """

    # Syntax for including a header with extra method-definitions (e.g.
    # hand-written).
    extra_include = r'/\* extra_include: \"(.*?)\" \*/'

    # Lenient syntax for the method markup, to allow providing
    # feedback when comments that look like method/function markup
    # contain errors.
    function_lenient = r'/\*\s?(method|function):\s*?.*?\*/'

    # Strict syntax applied to markup matching the lenient syntax to
    # discover errors.
    function_strict = r'/\*[ ](method|function):[ ]\"(.*?)\"( |\nname: \"(.*?)\" )\*/'

    # The actual regex used to match markup of methods and functions.
    function = r'/\* (method|function): \"(.*?)\"(?: |(?:\nname: \"(.*?)\" ))\*/\n^(?:static[ ]|extern[ ]|template[<]typename[ ]T[>]\n^)(?:const[ ])?.*?[ ](.*?)\((.*?)\)'

    # Syntax for a property-comment (struct with setter and getter).
    property = r'/\* property: \"(.*?)\" \*/\n^struct (.*?)\{'


def _to_py_name(cpp_name, entry_type):
    """Returns the name the function should have in the Python api, based
    on the c++-function name.

    For entry_type 'function', the cpp_name is used unmodified,
    otherwise strip everything before the first underscore, so that
    e.g:
      > someclass_some_method(PyObject* self, ...)

    gets the name "some_method", and is invoked with:

      > some_object.some_method(...)

    """
    if entry_type == 'function':
        return cpp_name
    first_underscore = cpp_name.find('_')
    assert(first_underscore != -1)
    return cpp_name[first_underscore + 1:]


def _property_to_py_name(cpp_struct_name):
    """Returns the name the property should have in the Python api,
    based on the C++ struct name."""
    first_underscore = cpp_struct_name.find('_')
    assert first_underscore != -1
    return cpp_struct_name[first_underscore + 1:]


def _clean_doc(doc):
    """Remove spurious end-lines and such from the doc-string specified in
    the comment preceding the C++-python functions.

    """
    # Replace regular enter (i.e. mere comment formatting in cpp file)
    # with space
    doc = doc.replace("\n", " ")

    # The removal can cause a "hard enter" (literal \n) to get an unintended
    # trailing space - trim those.
    doc = doc.replace("\\n ", "\\n")
    return doc


def _get_type(args_str, entry_type):
    """Determines the Python method type (METH_NOARGS or METH_VARARGS)
    from the C++ argument list and type of function.

    """
    # The C-method-implementations accept self as the first argument,
    # so a one-argument method will be invoked with zero arguments in Python.
    no_args = 1 if entry_type == "method" else 0
    return ("METH_NOARGS" if len(args_str.split(",")) == no_args
            else "METH_VARARGS")


def _find_extra_include(file_name):
    """Find any comment for including additional method definitions (e.g.
    hand-written for special cases) inside the generated method-defs
    array.
    The comment format is:
      > /* extra_include "<path>" */

    """
    extra_includes = []
    with open(file_name) as f:
        for m in re.finditer(regex.extra_include, f.read()):
            extra_includes.append(m.groups(1))
    return extra_includes


def check_file(file_name):
    """Uses a more permissive variant of the comment-regex used to
    recognize C++ methods to catch malformed comments. Throws
    value error if anything problematic is discovered.

    """
    result = []
    with open(file_name) as f:
        text = f.read()

    for m in re.finditer(regex.function_lenient, text, re.DOTALL|re.MULTILINE):
        m2 = re.match(regex.function_strict,
            m.group(0), re.DOTALL|re.MULTILINE)

        if not m2:
            raise ValueError("Invalid method markup: %s" % m.group(0))

        if m2.group(2).find('"') != -1:
            raise ValueError('Extra quote (") in method markup: %s' %
            m.group(0))

        result.append(m.group(0))
        end = m.end(0)
        if not text[end] == '\n':
            raise ValueError("Missing endline after method markup.")

        # Verify that the function is static.
        # This helps ensure that all functions are included in the
        # method-def (otherwise the compiler warns about unused static
        # function).
        #
        # Regular Faint functions aren't made specifically for a methoddef,
        # and may have external linkage (e.g. bitmap draw functions),
        # so allow extern as well, but require stating so explicitly.
        static = text[end+1:].startswith("static")
        extern = text[end+1:].startswith("extern")
        template = text[end+1:].startswith("template<typename T>")
        if not static and not extern and not template:
            rest = text[end+1:]
            methodLine = rest[:rest.find('\n')]
            raise ValueError("Marked up method not static or extern:\n%s "
                % methodLine[:-1])
    return result


def parse_file(file_name):
    """Finds all...
    C-functions marked up with:
      > /* <type>: "<doc_string>" */
      > static/extern ...

    and structs with getters and setters marked up with:
      > /* property: "<doc_string>

    Returns two lists.

    The method list contains:
    1) type ('method' or 'function')
    2) cpp-name: The name of the C++ function
    3) method-type: METH_VARARGS or METH_NOARGS
    4) The Python name
    5) The doc-string (including markup)

    The property list contains:
    1) The name of the c++ struct
    2) The name the property should have in Python
    3) The doc-string (including markup)

    """
    result = [[],[]]
    with open(file_name) as f:
        text = f.read()

    # Methods
    for m in re.finditer(regex.function, text, re.DOTALL|re.MULTILINE):

        entry_type, doc, py_name, cpp_name, args = m.groups()
        if m.group(0).find("template") != -1:
            assert entry_type == 'method'
            entry_type = 'method_template'

        if py_name is None:
            py_name = _to_py_name(cpp_name, entry_type)
        result[0].append((entry_type, cpp_name, _get_type(args, entry_type),
          py_name, _clean_doc(doc)))

    # Properties
    for m in re.finditer(regex.property, text, re.DOTALL|re.MULTILINE):
        doc, cpp_struct_name = m.groups()
        py_name = _property_to_py_name(cpp_struct_name)
        result[1].append((cpp_struct_name, py_name, _clean_doc(doc)))
    return result


def _to_PyMethodDef_entry(items):
    """Creates one entry for a PyMethodDef array from the entries for one
    function (as returned by parse_file).

    """

    entry_type = items[0]
    items = items[1:]
    if entry_type == 'method':
        return 'FORWARDER(%s, %s, "%s", "%s")' % items
    elif entry_type == 'function':
        return 'FREE_FORWARDER(%s, %s, "%s", "%s")' % items
    elif entry_type == 'method_template':
        return 'FORWARDER(%s<common_type>, %s, "%s", "%s")' % items
    else:
        assert False

def to_PyGetSetDef_entry(cpp_struct_name, py_name, doc):
    """Creates one entry for a PyGetSetDef array from the entries for one
    property-struct (as returned by parse_file).

    """
    return 'PROPERTY_FORWARDER(%s, "%s", "%s")' % (
        cpp_struct_name, py_name, doc)


def _to_PyGetSetDef(name, entries):
    """Creates a string of a C-PyGetSetDef array named <name>_getseters,
    containing all entries in the list (as created by
    to_PyGetSetDef_entry).

    """
    getSetDefEntries = [to_PyGetSetDef_entry(*items) for items in entries]
    getSetDef = ('static PyGetSetDef %s_getseters[] = {\n  ' % name +
                 ',\n  '.join(getSetDefEntries) + ',\n  ')
    getSetDef += '{nullptr,nullptr,nullptr,nullptr,nullptr} // Sentinel\n};'
    return getSetDef


def _to_PyMethodDef(name, entries, extra_includes):
    """Creates a string of a C-PyMethodDef array named <name>_methods,
    containing all the entries in the list (as created by
    _to_PyMethodDef_entry).

    Includes any include in the extra_includes list after the regular
    entries (before the sentinel).

    """

    methodEntries = [_to_PyMethodDef_entry(items) for items in entries]
    if name is not None:
        methodDef = ('static PyMethodDef %s_methods[] = {\n  ' % name +
            ',\n  '.join(methodEntries) + ',\n  ')
    else:
        methodDef = ',\n'.join(methodEntries) + ',\n  '

    for include in extra_includes:
        methodDef += '#include "%s"\n' % include

    if name is not None:
        methodDef += '{nullptr,nullptr,0,nullptr} // Sentinel\n};'
    return methodDef


def _write_result(file_name, name, entries, extra_includes, src_file_names):
    """Writes a PyMethodDef array and/or a PyGetSetDef to the specified
    file, featuring entries from the entries lists (as returned by
    parse_file).

    """

    with open(file_name, 'w', newline='\n') as f:
        f.write('// Generated by %s\n' % os.path.basename(__file__))
        f.write('// Based on %s: %s\n' %
                (("this file" if len(src_file_names) < 2 else
                 "these files"), ", ".join(src_file_names)))
        methods = entries[0]
        if len(methods) != 0:
            f.write(_to_PyMethodDef(name, methods, extra_includes))
            f.write('\n')

        properties = entries[1]
        if len(properties) != 0:
            f.write('\n')
            f.write(_to_PyGetSetDef(name, properties))


def _doc_to_html(doc):
    """Makes the doc-string more suitable for html."""
    return (doc.replace('\\n','<br>')
            .replace('->','&rarr;')
            .replace('...', '&#8230;')
            .replace('\\\\', '\\'))


def _write_method_doc(file_name, entries):
    """Writes an html-file documenting the passed in methods, using the
    doc-strings (as returned by parse_file)

    """

    with open(file_name, 'w', newline='\n') as f:
        f.write('<table border="0">')
        f.write('<tr><td><b>Method</b></td><td><b>Description</b></td></tr>')
        for items in entries:
            f.write('<tr><td valign="top">%s</td><td>%s</td></tr>' %
                    (items[3], _doc_to_html(items[4])))
        f.write('</table>')


def write_property_doc(file_name, entries):
    """Writes an html-file documenting the passed in properties, using the
    doc-strings (as returned by parse_file)

    Expects a list of (property-name, doc-str)-tuples.

    """
    if len(entries) == 0:
        return

    with open(file_name, 'w', newline='\n') as f:
        f.write('<!-- Generated by %s -->' % os.path.basename(__file__))
        f.write('<table border="0">')
        f.write('<tr><td><b>Property</b></td><td><b>Description</b></td></tr>')
        for items in entries:
            f.write('<tr><td valign="top">%s</td><td>%s</td></tr>' %
                    (items[1], _doc_to_html(items[2])))
        f.write('</table>')


def generate(src_file_names,
             dst_file_name,
             dst_doc_file_name,
             dst_property_doc_file_name,
             name):
    """Generate the Python methoddef header and html documentation for the
    c++-file indicated by src_file_name, by locating "special"
    C-comments.

    The header is saved to dst_file_name and the html documentation to
    dst_doc_file_name. The name is used for the PyMethodDef and PyGetSetDef.

    """
    methods = []
    properties = []
    extra_includes = []
    entries = (methods, properties)
    for src_file_name in src_file_names:
        check_file(src_file_name)
        m, p = parse_file(src_file_name)
        methods.extend(m)
        properties.extend(p)

        extra_includes.extend(_find_extra_include(src_file_name))
        if len(entries[0]) == 0 and len(entries[1]) == 0:
            print("No entries found in %s." % src_file_name)
            exit(1)

    _write_result(dst_file_name, name, entries, extra_includes, src_file_names)
    _write_method_doc(dst_doc_file_name, entries[0])
    write_property_doc(dst_property_doc_file_name, entries[1])


def clean(src_files, doc_root):
    for (name, files) in src_files:
        src = files if files.__class__  == str else files[0]

        dst = src.replace(".hh", "-methoddef.hh")
        dst = dst.replace(".cpp", "-methoddef.hh")
        dst_doc = src.replace(".cpp", '-methods.txt')
        dst_doc = dst_doc.replace(".hh", '-methods.txt')
        dst_doc_filename = os.path.split(dst_doc)[1]
        dst_doc_filename = os.path.join(doc_root, dst_doc_filename)
        if os.path.exists(dst):
            os.remove(dst)
        if os.path.exists(dst_doc_filename):
            os.remove(dst_doc_filename)


def generate_headers(src_files, out_root, doc_root):
    """Generate headers with a Python methoddef array and html
    documentation tables for the listed source files.

    The list should contain tuples of names and paths:
      [(desired-method-def-name, cpp-file-path),...]

    The name is used for the generated method-def:
      static PyMethodDef <name>_methods[].

    doc_root indicates the folder where the generated
    html-documentation should be stored.

    The generated header will be named the same as the source file,
    but with .cpp stripped and -methoddef.hh appended.

    The html file will be named the same as the source-file but
    with .cpp stripped and -methods.txt appended."

    """

    if not os.path.exists(out_root):
        os.makedirs(out_root)
    did_print_heading = False
    changed = False
    for (name, files) in src_files:
        if files.__class__  == str:
            src = files
            files = (src,)
        else:
            src = files[0]

        dst = src.replace(".hh", "-methoddef.hh")
        dst = dst.replace(".cpp", "-methoddef.hh")
        dst = os.path.join(out_root, os.path.split(dst)[1])

        dst_doc = src.replace(".hh", '-methods.txt')
        dst_doc = dst_doc.replace(".cpp", '-methods.txt')
        dst_doc_filename = os.path.split(dst_doc)[1]
        dst_doc_filename = os.path.join(doc_root, dst_doc_filename)

        dst_prop_doc = src.replace(".cpp", '-properties.txt')
        dst_doc_prop_filename = os.path.split(dst_prop_doc)[1]
        dst_doc_prop_filename = os.path.join(doc_root, dst_doc_prop_filename)

        if util.changed(src, dst):
            if not did_print_heading:
                print("* Generating Python method definitions.")
                did_print_heading = True
            generate(files, dst, dst_doc_filename, dst_doc_prop_filename, name)
            changed = True
    if not changed:
        print("* Python method definitions up to date.")
