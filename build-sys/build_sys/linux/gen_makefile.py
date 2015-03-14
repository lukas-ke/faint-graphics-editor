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

def compiler_specific_warnings(cc):
    if cc == 'gcc' or cc == 'g++':
        return ' '.join(
            [
                '-Wuseless-cast',
                #'-Wzero-as-null-pointer-constant',
            ])
    else:
        return ''
        
def gen_makefile(fileList, opts, debug, cc):
    makefile_name = "generated_makefile"
    objRoot = opts.get_obj_root()
    makefile = open(makefile_name, 'w')

    includes = [ "-isystem " +
        include for include in opts.system_include_folders]
    includes.extend([
        "-I%s" % include for include in opts.include_folders])

    makefile.write("INCLUDES=%s\n" % " \\\n  ".join(includes))

    if debug:
        makefile.write("CCFLAGS=-Wall -Wextra -Werror -pedantic -ansi -Wconversion -Wno-strict-aliasing -Wno-sign-conversion -std=c++14 -g -c %s\n" % 
                       compiler_specific_warnings(cc))        
    else:
        makefile.write("CCFLAGS=-Wall -Wextra -Werror -pedantic -ansi -Wconversion -O2 -Wno-sign-conversion -Wno-strict-aliasing -std=c++14 -Wunused -fdiagnostics-show-option -c %s\n" %
                       compiler_specific_warnings(cc))

    makefile.write("WXFLAGS:=$(shell %s/wx-config --cxxflags)\n" % opts.wx_root)
    makefile.write("\n")
    makefile.write("all: " + " \\\n  ".join([os.path.join(objRoot, os.path.basename(f).replace(".cpp", ".o")) for f in fileList if '/msw/' not in f])) # Fixme: Ugly way to exclude msw
    makefile.write('\n\n')

    for f in fileList:
        if '/msw/' in f: # Fixme: Ugly way to exclude msw
            continue
        objName = os.path.join(objRoot, os.path.basename(f).replace(".cpp", ".o"))
        makefile.write('%s: %s\n' % (objName, f))
        makefile.write('\t%s $(CCFLAGS) $(INCLUDES) $(WXFLAGS) $< -o $@\n' % cc)
        makefile.write('\n')

    return makefile_name
