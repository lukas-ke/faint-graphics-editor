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

from optparse import OptionParser
from build_sys.util.util import print_timing
import build_sys.util.util as util
import os
import sys
import build_sys.gen_build_info as gen_build_info
import build_sys.gen_help as gen_help
import build_sys.gen_nsis as gen_nsis
import build_sys.dependencies as depend
from os.path import abspath

unknown_version_str = "unknown"

def fail(message):
    print("Error: %s" % message)
    exit(1)

def parse_bool(config, group, name):
    value = config.get(group, name)
    if value.lower() in ['1', 'true', 'yes', 'y']:
        return True
    if value.lower() in ['0', 'false', 'no', 'n']:
        return False
    print("Error: %s should be 1 or 0 in build.cfg" % name)
    exit(1)

def parse_command_line():
    optParser = OptionParser()
    optParser.add_option("", "--rebuild",
                         action="store_true",
                         dest="do_rebuild",
                         default=False,
                         help="Rebuild and relink")

    optParser.add_option("", "--version",
                         dest="version",
                         default=unknown_version_str,
                         help="Application version number")

    optParser.add_option("", "--debug",
                         action="store_true",
                         dest="debug",
                         default=False,
                         help="Build with debug symbols")

    optParser.add_option("", "--filthy",
                         action="store_true",
                         dest="filthy",
                         default=False,
                         help="Allow building unclean installer")

    optParser.add_option("", "--timed",
                         action="store_true",
                         dest="timed",
                         default=False,
                         help="Time the build")

    optParser.add_option("", "--stdout",
                         action="store_true",
                         dest="use_stdout",
                         default=False,
                         help="Print compilation output on standard out")

    return optParser.parse_args()


def clean_file_name(f):
    return abspath(f).replace('\\', '/')


def versioned(cmd_opts):
    return cmd_opts.version != unknown_version_str


def versioned_unfilthy(cmd_opts):
    return versioned(cmd_opts) and not cmd_opts.filthy


def check_valid_release(cmd_opts, opts, clean_build):
    if not clean_build:
        if versioned_unfilthy(cmd_opts):
            fail("Build with --version requires empty output folder "
                 "(build/objs)")

    if versioned_unfilthy(cmd_opts):
        if gen_build_info.working_copy_modified(opts.project_root):
            fail("Build with --version requires unmodified working copy.")

    if versioned(cmd_opts) and opts.makensis_exe is None:
        fail("Build with --version requires path to nsis in build.cfg")


def prepare_out_dir(obj_root):
    """Recreates the output-dir. Returns true if the folder was recreated
    or empty. (this signifies a clean build)"""
    if not os.path.exists(obj_root):
        os.mkdir(obj_root)
        return True
    elif len(os.listdir(obj_root)) == 0:
        return True
    else:
        return False


def _modified_directly(source_files, obj_root, obj_ext):
    """Returns the cpp-files which are modified more recently than their
    object files.

    """
    modified = []
    for cpp in source_files:
        obj_file = os.path.join(obj_root,
                                os.path.split(cpp)[1].replace('.cpp',
                                                              obj_ext))
        if util.changed(cpp, obj_file):
            modified.append(cpp)

    return set(modified)


def _to_obj(obj_root, cpp, obj_ext):
    return os.path.join(obj_root,
        os.path.basename(cpp).replace('.cpp', obj_ext))


def _modified_dependencies(deps, obj_root, obj_ext, ignore):
    """Returns the cpp-files for which an object file is older than a
    header it depends on.

    """
    modified_deps = []
    for incl in deps:
        for cpp in deps.get(incl, []):
            cpp = clean_file_name(cpp)

            objFile = _to_obj(obj_root, cpp, obj_ext)
            if cpp not in ignore and util.changed(incl, objFile):
                modified_deps.append(cpp)

    return set(modified_deps)


def build(opts, cmdline):
    """Build according by the build options and command line options and
    arguments.

    """

    # Check that the parameters are valid
    opts.verify()

    if opts.compiler is None:
        fail("No compiler specified")

    if opts.compiler == 'msvc':
        import build_sys.compile_msw as compile_impl
        compile = compile_impl.compile
        link = compile_impl.link

    else:
        import build_sys.compile_linux as compile_impl
        if opts.compiler == 'clang':
            compile = compile_impl.compile_clang
            link = compile_impl.link_clang
        elif opts.compiler == 'gcc':
            compile = compile_impl.compile_gcc
            link = compile_impl.link_gcc
        elif opts.compiler == 'iwyu':
            compile = compile_impl.compile_iwyu
            link = compile_impl.link_iwyu
        else:
            fail("Unsupported compiler (%s) specified." % opts.compiler)

    obj_ext = compile_impl.obj_ext
    create_installer = compile_impl.create_installer

    cmd_opts, args = cmdline

    opts.set_debug_compile(cmd_opts.debug)
    objRoot = opts.get_obj_root()
    opts.timed = cmd_opts.timed

    clean_build = prepare_out_dir(objRoot)
    check_valid_release(cmd_opts, opts, clean_build)

    do_compile = True
    do_link = True
    do_rebuild = cmd_opts.do_rebuild

    err = open('err.txt', 'w')
    if cmd_opts.use_stdout:
        out = sys.stdout
    else:
        out = open('out.txt', 'w')

    modified = ()
    to_compile = set()
    depsChanged = set()

    timed = util.timed if opts.timed else util.not_timed

    if opts.check_deps:
        print("* Generating dependencies.")
        deps = timed(depend.get_flat_header_dependencies, opts.project_root,
                     opts.source_folders)
    else:
        deps = []

    modified = set()
    depModified = set()

    opts.source_files = list(set([os.path.abspath(f) for f in opts.source_files]))
    opts.source_files = [clean_file_name(f) for f in opts.source_files]

    compile_unchanged = False

    if do_compile:
        if do_rebuild:
            to_compile = opts.source_files
        else:
            modified = _modified_directly(opts.source_files, objRoot, obj_ext)
            to_compile = to_compile.union(modified)

            depModified = _modified_dependencies(deps, objRoot, obj_ext,
                                                 ignore=to_compile)
            to_compile = to_compile.union(depModified)

    if len(to_compile) > 0:
        if opts.create_build_info:
            print("* Generating Build-info.")
            to_compile.add(clean_file_name(gen_build_info.run(opts.project_root,
                                                              cmd_opts.version)))

        print("* Compiling.")
        print('  %d cpp files modified + %d indirectly (of %d total).' %
              (len(modified), len(depModified), len(opts.source_files)))
        timed(compile, list(sorted(list(to_compile))), opts,
                out, err, debug=cmd_opts.debug)
    else:
        print("* Nothing to compile.")
        compile_unchanged = True

    obj_files = util.joined(opts.get_obj_root(),
                       util.list_by_ext(opts.get_obj_root(), obj_ext))
    obj_files.extend([o + obj_ext for o in opts.extra_objs])


    if compile_unchanged:
        if util.any_changed(obj_files, opts.get_out_path()):
            do_link = True
        else:
            print("* Not relinking either.") # Fixme: Tidy up
            do_link = False

    if do_link:
        print("* Linking (%d object files)." % len(obj_files))

        timed(link, obj_files, opts, out, err, debug=cmd_opts.debug)

    if opts.platform == 'msw' and cmd_opts.version != unknown_version_str:
        print("* Generating %s installer." % cmd_opts.version)
        oldDir = os.getcwd()
        os.chdir("../installer")
        nsiFile = gennsis.run(cmd_opts.version)
        create_installer(opts.makensis_exe, nsiFile)
        os.chdir(oldDir)
    return 0

if __name__ == '__main__':
    build()
