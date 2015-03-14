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
import subprocess
from . linux.gen_makefile import gen_makefile
obj_ext = '.o'

def _compile(fileList, opts, out, err, debug, cc):
    makefile_name = gen_makefile(fileList, opts, debug, cc)

    make_cmd = 'make -f %s' % makefile_name
    if opts.parallell_compiles != 0:
        make_cmd += ' -j %d' % opts.parallell_compiles
    if cc == 'iwyu':
        make_cmd += ' -k'
    print(make_cmd)
    make = subprocess.Popen(make_cmd, 0, None, None, out, err, shell=True)
    if make.wait() != 0:
        print("Compilation failed")
        exit(1)

def _get_wxlibs( wxRoot ):
    wxcfg = subprocess.Popen( "%s/wx-config --libs" % wxRoot, 0, shell=True, stdout=subprocess.PIPE )
    return wxcfg.communicate()[0].strip().decode("ascii") # Fixme

def _link(files, opts, out, err, debug, cc):
    old = os.getcwd()
    os.chdir(opts.project_root)
    wxlibs = _get_wxlibs(opts.wx_root)
    out_name = opts.get_out_name()
    lib_paths = " ".join(["-L%s" % p for p in opts.lib_paths])

    cmd = (cc + " -std=c++11 -g -o %s " % out_name +
           " ".join(files) + " " + wxlibs + " " + lib_paths +
           " -l python3.4 -O2")

    linker = subprocess.Popen(cmd, stdout=out, stderr=err, shell=True)
    if linker.wait() != 0:
        print("Linking failed")
        exit(1)
    os.chdir( old )

def create_installer(*arg, **kwarg):
    assert(False)


def compile_gcc(fileList, opts, out, err, debug):
    _compile(fileList, opts, out, err, debug, cc='g++')

def link_gcc(files, opts, out, err, debug):
    _link(files, opts, out, err, debug, cc='g++')

def compile_clang(fileList, opts, out, err, debug):
    _compile(fileList, opts, out, err, debug, cc='clang++')

def compile_iwyu(fileList, opts, out, err, debug):
    _compile(fileList, opts, out, err, debug, cc='iwyu')

def link_iwyu(fileList, opts, out, err, debug):
    pass

def link_clang(files, opts, out, err, debug):
    _link(files, opts, out, err, debug, cc='clang++')
