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

import os

CC_OPTIONS_COMMON = [
    "Wall",
    "Wextra",
    "Werror", # Treat warnings as errors
    "pedantic",
    "ansi",
    "Wno-strict-aliasing", # No aliasing warnings
    "Wno-sign-conversion", # No sign conversion warnings
    "std=c++14", # C++14-conformance
    "c", # Do not invoke linker
]

CC_OPTIONS_DEBUG = [
    "g",
]

CC_OPTIONS_RELEASE = [
    "O2", # Optimization level
]

GCC_SPECIFIC_WARNINGS = [
    #'Wuseless-cast',
    #'Wzero-as-null-pointer-constant',
    #'Wconversion',
]

CLANG_SPECIFIC_WARNINGS = [
    'Wconversion',
]

def compiler_specific_warnings(cc):
    if cc in ('gcc', 'g++'):
        return GCC_SPECIFIC_WARNINGS
    else:
        return CLANG_SPECIFIC_WARNINGS


def get_cc_options_list(cc, debug):
    options = CC_OPTIONS_COMMON[:]
    if debug:
        options.extend(CC_OPTIONS_DEBUG)
    else:
        options.extend(CC_OPTIONS_RELEASE)
        options.extend(compiler_specific_warnings(cc))
    return options


def get_includes_list(opts):
    includes = ["-isystem " + include
                 for include in opts.system_include_folders]
    includes.extend(['-I%s' % include
                     for include in opts.include_folders])
    return includes


def format_options(options):
    return " ".join(['-%s' % opt for opt in options])


def str_CCFLAGS(cc, debug):
    return ('CCFLAGS=%s\n\n' %
            format_options(get_cc_options_list(cc, debug)))


def str_INCLUDES(includes):
    return ("INCLUDES=%s\n" %
            " \\\n  ".join(includes))


def str_WXFLAGS(opts):
    return ("WXFLAGS:=$(shell %s/wx-config --cxxflags)\n\n" %
            opts.wx_root)


def o_from_cpp(f):
    b = os.path.basename(f)
    b = b.replace(".cpp", ".o")
    b = b.replace(".c", ".o")
    return b


def str_target_all(cppFiles, objRoot):
    objFiles = [os.path.join(objRoot, o_from_cpp(f))
                      for f in cppFiles]

    objFilesStr = " \\\n  ".join(objFiles)

    return "all: %s\n\n" % objFilesStr


def str_target_obj(objRoot, cppFile, cc):
    objName = os.path.join(objRoot, o_from_cpp(cppFile))
    return ('%s: %s\n' % (objName, cppFile) +
            '\t%s $(CCFLAGS) $(INCLUDES) $(WXFLAGS) $< -o $@\n\n' % cc)


def without_msw(fileList):
    return [f for f in fileList if "/msw/" not in f]


def gen_makefile(fileList, opts, debug, cc):
    makefile_name = "generated_makefile"
    objRoot = opts.get_obj_root()
    cppFiles = without_msw(fileList)

    with open(makefile_name, 'w') as makefile:
        makefile.write(str_CCFLAGS(cc, debug))
        makefile.write(str_INCLUDES(get_includes_list(opts)))
        makefile.write(str_WXFLAGS(opts))
        makefile.write(str_target_all(cppFiles, objRoot))

        for f in cppFiles:
            makefile.write(str_target_obj(objRoot, f, cc))

        return makefile_name
