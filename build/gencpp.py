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

import sys
from collections import namedtuple
import os

if __name__ == '__main__':
    sys.path.append("../build-sys/");

import build_sys.gen_method_def as gen_method_def
import build_sys.cpp_writer as cpp

"""Generates the setting-modification functions and properties in the
Faint Python API.

"""


def _should_generate(sources, out_dir):
    """True if the source-files are modified more recently than the output
    files

    """
    # Fixme: Rework. Use smarter file-name discovery
    outfiles = [os.path.join(out_dir, f) for f in
                ["setting-function-defs.hh",
                 "setting-functions.hh",
                 "py-settings-properties.hh"]]
    newest = max([os.path.getmtime(f) for f in sources])

    for f in outfiles:
        if not os.path.isfile(f) or os.path.getmtime(f) < newest:
            return True
    return False


def filename_to_varname(filename):
    """Converts a template file-name to a python variable name"""
    return filename.lower().replace('-','_').replace('.txt','')


def preprocess(line, filename):
    """Performs initial replacements while reading a template"""
    return line.replace('$$file', filename)


def load_templates(root):
    """Load all text files in templates/ and return an object with lists
    of the lines.

    """

    filenames = [f for f in os.listdir(root) if f.endswith('.txt')]
    names = [filename_to_varname(f) for f in filenames]
    TemplateTuple = namedtuple("TEMPLATES", names)

    templates = []
    for filename in filenames:
        with open(os.path.join(root, filename)) as f:
            lines = [preprocess(l, filename) for l in f.readlines()]
            templates.append(lines)

    return TemplateTuple(*templates)


# Contains fields with the texts from all text files in the given
# folder.
#
# The fields are named like the file they represent, but
# without extension and with dash replaced by underscore.
# > function-int.txt -> function_int
TEMPLATES = load_templates('../python/generate/templates')


def common_replacements(setting, item):
    """Maps keys to values from setting and item for replacing string
    templates.

    """
    return {"$name" : setting.lower(),
            "$setting" : setting,
            "$prettyname" : item.py_name,
            "$doc_str" : item.doc_str}


def numeric_replacements(setting, item):
    """Maps keys to values from setting and item for replacing string
    templates for settings with numeric ranges.

    """
    replacements = common_replacements(setting, item)
    replacements.update({
        "$min_value" : str(item.min_value),
        "$max_value" : str(item.max_value)})
    return replacements


def string_to_int_replacements(setting, item, for_property):
    """Maps keys to values from setting and item for replacing string
    templates for settings which need to be converted to/from Strings.

    """
    replacements = common_replacements(setting, item)
    replacements.update({
        '$string_to_int' : string_to_int(item, property=for_property),
        '$int_to_string' : int_to_string(item)})
    return replacements


def non_shorthand_keys(py_to_cpp_map):
    """Retrieve the keys but ignore all shorthand forms, i.e., ignore keys
    that would translate a value like TRANSPARENT to a short form like
    't'

    """
    return [ key for key in py_to_cpp_map.keys() if
             py_to_cpp_map[key].__class__ == "".__class__ ]


def get_value(map, key):
    """Returns the value part from the dict, ignoring shorthand variants.

    """
    if map[key].__class__ == "".__class__:
        return map[key]
    else:
        return map[key][0]


def int_to_string(item):
    """Uses the py_to_cpp_map to create a block of code turning the
    integer (representing an enum setting value) into a string.

    """

    py_to_cpp_map = item.py_to_cpp_map
    keys = list(non_shorthand_keys(py_to_cpp_map))
    key = keys[0]
    value = py_to_cpp_map[key]
    cc = cpp.Code()

    # First if
    cc += 'if (i_value == to_int(%s)){' % value
    cc += 's_value = "%s";' % key
    cc += '}'

    # Each else if
    for key in keys[1:]:
        value = py_to_cpp_map[key]
        cc += 'else if (i_value == to_int(%s)){' % value
        cc += 's_value = "%s";' % key
        cc += '}'

    # Final else (error case)
    cc += 'else{'
    cc += 'throw ValueError("Unknown value");'
    cc += '}'
    return cc.get_text()


def string_to_int(item, property):
    """Uses the py_to_cpp_map to create a block of code turning the
    string parameter into an integer (representing a setting enum).

    """
    # Convert to a block of code turning the parameter into an integer

    py_to_cpp_map = item.py_to_cpp_map
    keys = list(py_to_cpp_map.keys())

    key = keys[0]
    value = get_value(py_to_cpp_map, key)
    cc = cpp.Code()

    # First if
    cc += '%s value = %s::MIN;' % (item.cpp_enum.name, item.cpp_enum.name)
    cc += 'if (s_value == utf8_string("%s")){' % key
    cc += 'value = %s;' % value
    cc += '}'

    # Each else if
    for key in keys[1:]:
        value = get_value(py_to_cpp_map, key)
        cc += 'else if (s_value == utf8_string("%s")){' % (key)
        cc += 'value = %s;' % value
        cc += '}'

    # Defaulting else (error case)
    cc += 'else{'
    cc += 'throw ValueError("Invalid string");'
    cc += '}'
    return cc.get_text()


def generate_function(cc, method_def, cc_template, method_def_template, replacements):
    cc.append_template(cc_template, replacements)
    cc.blank_line()
    method_def.append_template(method_def_template, replacements)


def generate_property(hh, cc, hh_template, cc_template, replacements):
    hh.append_template(hh_template, replacements)
    cc.append_template(cc_template, replacements)
    cc.line("")


def generate_setting_id_header(settings):
    ns = cpp.Namespace('faint')
    for setting_id in sorted(settings.keys()):
        item = settings[setting_id]
        if item.get_type() == 'stringtoint':
            enum = item.cpp_enum
            entries = list(enum.entries)
            entries.append("MIN=%s" % enum.entries[0])
            entries.append("MAX=%s" % enum.entries[-1])
            entries.append("DEFAULT=%s" % enum.entries[0])
            ns += 'enum class %s{' % enum.name
            for entry in entries[:-1]:
                ns += entry + ','
            ns += entries[-1]
            ns += '};'
            ns += ''
            ns += 'extern const %s %s;' % (item.cpp_type, item.cpp_name)
            ns += '%s to_%s(int);' % (enum.name, item.py_name)
            ns += 'bool valid_%s(int);' % (item.py_name)
            ns += 'int to_int(%s);' % (enum.name)
            ns += ''
        else:
            ns += 'extern const %s %s;' % (item.cpp_type, item.cpp_name)

    ns += 'utf8_string setting_name(const UntypedSetting&);'
    ns += 'utf8_string setting_name_pretty(const UntypedSetting&);'
    ns += ''
    ns += '// Returns the name for this value if the IntSetting has names for values'
    ns += '// otherwise, just returns the value as a string'
    ns += 'utf8_string value_string(const IntSetting&, int value);'
    ns += ''

    comment = cpp.Comment('Generated by %s' % os.path.basename(__file__))

    return comment + cpp.IncludeGuard('CPP_SETTING_ID_HH',
                                      cpp.Include('"util/settings.hh"') + ns)


def cpp_int_to_enum_impl(item):
    """Writes a C++-function for converting an integer value to the
    enumerated constant for the setting

    """
    enum = item.cpp_enum
    cc = cpp.Code()
    cc += '%s to_%s(int v){' % (enum.name, item.py_name)
    condition = 'if'

    for entry in enum.entries:
        cc += '%s (v == to_int(%s::%s)){' % (condition, enum.name, entry)
        cc += 'return %s::%s;' % (enum.name, entry)
        cc += '}'
        condition = 'else if'
    cc += 'assert(false);'
    cc += 'return %s::%s;' % (enum.name, enum.entries[0])
    cc += '}'
    cc += ''
    return cc


def cpp_enum_to_int_impl(item):
    """Returns a C++-function for casting an enum value to an integer

    """
    cc = cpp.Code()
    cc += 'int to_int(%s v){' % item.cpp_enum.name
    cc += 'return static_cast<int>(v);'
    cc += '}'
    cc += ''
    return cc


def setting_name_impl(settings, pretty):
    cc = cpp.Code()
    if pretty:
        cc += 'utf8_string setting_name_pretty(const UntypedSetting& s){'
    else:
        cc += 'utf8_string setting_name(const UntypedSetting& s){'

    condition = 'if'
    for setting_id in sorted(settings.keys()):
        item = settings[setting_id]
        cc += '%s (s == %s){' % (condition, item.cpp_name)
        cc += 'return "%s";' % (item.pretty_name if pretty
                                else item.py_name)
        cc += '}'
        condition = 'else if'
    cc += 'assert(false);'
    cc += 'return "undefined_setting_name";'
    cc += '}'
    cc += ''
    return cc


def cpp_value_to_key(map):
    cc = cpp.Code()
    condition = "if"
    for key in sorted(map.keys()):
        cc += '%s (value == to_int(%s)){' % (condition, key)
        cc += 'return "%s";' % map[key]
        cc += '}'
        condition = "else if"

    cc += 'assert(false);'
    cc += 'return "";'
    return cc


def inverse_py_to_cpp_map(map):
    """Inverse the mapping and exclude shorthand entries"""
    return {v:k for k,v in
            [item for item in map.items() if item[1].__class__ == str]}


def cpp_value_string_impl(settings):
    """Writes a C++-function for converting the value for an integer
    setting to its mnemonic if one is available

    """

    cc = cpp.Code()
    cc += 'utf8_string value_string(const IntSetting& setting, int value){'

    enum_settings = [s for s in settings if
                     settings[s].get_type() == "stringtoint"]

    condition = "if"
    for setting_id in enum_settings:
        item = settings[setting_id]
        cc += '%s (setting == %s){' % (condition, item.cpp_name)
        cc += cpp_value_to_key(inverse_py_to_cpp_map(item.py_to_cpp_map))
        cc += '}'
        condition = "else if"

    cc += 'std::stringstream ss;'
    cc += 'ss << value;'
    cc += 'return utf8_string(ss.str());'
    cc += '}'
    return cc


def cpp_valid_impl(item):
    """Writes a C++-function for checking if the integer matches a
    defined constant value for the setting.

    """
    enum = item.cpp_enum
    cc = cpp.Code()
    cc += 'bool valid_%s(int v){' % item.py_name
    cc += ('return to_int(%s::MIN) <= v && v <= to_int(%s::MAX);' %
           (enum.name, enum.name))
    cc += '}'
    cc += ''
    return cc


def generate_setting_id_impl(settings):
    ns = cpp.Namespace('faint')
    for setting_id in sorted(settings.keys()):
        item = settings[setting_id]
        ns += 'const %s %s;' % (item.cpp_type, item.cpp_name)
    ns += ''

    ns += setting_name_impl(settings, pretty=True)
    ns += setting_name_impl(settings, pretty=False)

    for setting_id in sorted(settings.keys()):
        item = settings[setting_id]
        if item.get_type() == 'stringtoint':
            ns += cpp_int_to_enum_impl(item)
            ns += cpp_enum_to_int_impl(item)
            ns += cpp_valid_impl(item)

    ns += cpp_value_string_impl(settings)
    comment = cpp.Comment('Generated by %s' % os.path.basename(__file__))
    return (comment +
            cpp.Include('<sstream>') + cpp.Include('"cpp-setting-id.hh"') +
            ns)


def run(root_dir, force=False):
    """Generate C++-Python code using the templates under the root-dir,
    and the Faint settings defined in set_and_get.py

    """

    oldDir = os.getcwd()
    os.chdir(root_dir)
    sys.path.append(os.getcwd())

    template_files = [os.path.join("templates", f)
                      for f in os.listdir("templates")]

    out_dir = '../../generated/python/settings'
    assert(os.path.exists("set_and_get.py"))
    sources = template_files[:]
    sources.append("set_and_get.py")
    up_to_date = not _should_generate(sources, out_dir)
    # Fixme: Also check e.g. py-something-properties

    if not force and up_to_date:
        print("* Python-C++ interface up to date.")
        os.chdir(oldDir)
        return
    print("* Generating Python-C++ interface.")

    import set_and_get

    setters_and_getters = set_and_get.setters_and_getters

    settings_declarations = cpp.Code()
    settings_cc = cpp.Code()

    smth_declarations = cpp.Code()
    smth_cc = cpp.Code()

    functions_cc = cpp.Code()
    function_method_def = cpp.Code()

    # List of property-name to property-docs tuples
    properties = []
    for setting in setters_and_getters:
        item = setters_and_getters[setting]
        setting_type = item.get_type()

        properties.append((None, item.py_name, item.doc_str))
        if setting_type == 'bool':
            replacements = common_replacements(setting, item)


            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_bool,
                              TEMPLATES.function_method_def,
                              replacements)

            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_bool,
                              replacements)

            generate_property(smth_declarations,
                              smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_bool,
                              replacements)


        elif setting_type == 'color':
            replacements = common_replacements(setting, item)

            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_color,
                              TEMPLATES.function_method_def,
                              replacements)

            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_color,
                              replacements)

            generate_property(smth_declarations,
                              smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_color,
                              replacements)

        elif setting_type == 'float':
            replacements = numeric_replacements(setting, item)

            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_float,
                              TEMPLATES.function_method_def,
                              replacements)

            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_float,
                              replacements)

            generate_property(smth_declarations,
                              smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_float,
                              replacements)

        elif setting_type == 'int':
            replacements = numeric_replacements(setting, item)

            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_int,
                              TEMPLATES.function_method_def,
                              replacements)

            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_int,
                              replacements)

            generate_property(smth_declarations,
                              smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_int,
                              replacements)

        elif setting_type == 'string':
            replacements = common_replacements(setting, item)

            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_string,
                              TEMPLATES.function_method_def,
                              replacements)

            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_string,
                              replacements)

            generate_property(smth_declarations,
                              smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_string,
                              replacements)

        elif setting_type == 'stringtoint':
            replacements = string_to_int_replacements(setting, item,
                                                      for_property=False)
            generate_function(functions_cc,
                              function_method_def,
                              TEMPLATES.function_string_to_int,
                              TEMPLATES.function_method_def,
                              replacements)

            replacements = string_to_int_replacements(setting, item,
                                                      for_property=True)
            generate_property(settings_declarations,
                              settings_cc,
                              TEMPLATES.settings_property,
                              TEMPLATES.settings_string_to_int,
                              replacements)

            generate_property(smth_declarations, smth_cc,
                              TEMPLATES.object_property,
                              TEMPLATES.object_string_to_int,
                              replacements)
        else:
            print(setting_type)
            assert(False)

    property_doc_dir = '../../help/source/generated/'
    if not os.path.exists(property_doc_dir):
        os.mkdir(property_doc_dir)
    gen_method_def.write_property_doc(os.path.join(property_doc_dir,
      'py-settings-properties.txt'), properties)

    comment = cpp.Comment('Generated by %s' %
                                    os.path.basename(__file__))

    interface_cc = (comment +
                    cpp.Include('"python/py-include.hh"') +
                    cpp.Include('"app/canvas.hh"') +
                    cpp.Include('"app/get-app-context.hh"') +
                    cpp.Include('"util/settings.hh"') +
                    cpp.Include('"util/setting-id.hh"') +
                    cpp.Include('"python/py-util.hh"') +
                    cpp.Include('"text/formatting.hh"') +
                    cpp.Include('"python/py-function-error.hh"') +
                    cpp.Namespace('faint', functions_cc))

    if not os.path.exists(out_dir):
        os.makedirs(out_dir)


    with open(os.path.join(out_dir, 'setting-functions.hh'), 'w') as f:
        f.write(interface_cc.get_text())

    with open(os.path.join(out_dir, 'cpp-setting-id.hh'), 'w') as f:
        f.write(generate_setting_id_header(setters_and_getters).get_text())

    with open(os.path.join(out_dir, 'cpp-setting-id.cpp'), 'w') as f:
        f.write(generate_setting_id_impl(setters_and_getters).get_text())

    with open(os.path.join(out_dir, 'py-settings-properties.hh'), 'w') as f:
        getset = cpp.Code()
        getset += 'static PyGetSetDef settings_properties[] ={'
        getset += settings_declarations
        getset += '{nullptr,nullptr,nullptr,nullptr,nullptr}'
        getset += '};'

        content = comment + cpp.IncludeGuard('FAINT_PY_SETTINGS_PROPERTIES_HH',
            cpp.Namespace('faint', settings_cc +
                          getset))
        f.write(content.get_text())



    # Fixme: Add hand-written properties in a less ugly way
    cpp_properties = gen_method_def.parse_file(os.path.abspath(
        os.path.join(root_dir, "../", "../", "py-something-properties.hh")))[1]

    getset = cpp.Code()
    getset += 'static PyGetSetDef smth_properties[] ={'
    getset += smth_declarations

    for item in cpp_properties:
        getset += gen_method_def.to_PyGetSetDef_entry(*item) + ","

    getset += '{nullptr,nullptr,nullptr,nullptr,nullptr}'
    getset += '};'

    content = (
        comment +
        cpp.IncludeGuard('FAINT_PY_SMTH_SETTING_PROPERTIES_HH',
            cpp.Namespace('faint',
                smth_cc + getset)))

    with open(os.path.join(out_dir, 'py-smth-setting-properties.hh'), 'w') as f:
        f.write(content.get_text())

    gen_method_def.write_property_doc(
        os.path.join(property_doc_dir, 'py-something-properties.txt'),
        cpp_properties)

    with open(os.path.join(out_dir, 'setting-function-defs.hh'), 'w') as f:
        content = comment + cpp.IncludeGuard('FAINT_SETTING_FUNCTION_DEFS_HH',
            function_method_def)
        f.write(content.get_text())

    os.chdir(oldDir)

if __name__ == '__main__':
    run('../python/generate', force=True)
