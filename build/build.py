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

from optparse import OptionParser
import configparser
import os
join_path = os.path.join

import subprocess
import sys

build_dir = os.path.split(os.path.realpath(__file__))[0]
os.chdir(build_dir) # Fixme: Don't change dir, use absolute paths.
root_dir = os.path.split(build_dir)[0]

# Make build-system and test-system imports available
sys.path.append(join_path(root_dir, "build-sys/"))
sys.path.append(join_path(root_dir, "test-sys/"))
import build_sys as bs
from build_sys.util import list_cpp, strip_ext
from test_sys import gen_runner
import gencpp
import faint_info


def recreate_config(platform):
    f = open( "build.cfg" ,'w')
    f.write("[folders]\n")
    f.write("wx_root=\n")
    f.write("cairo_include=\n")
    f.write("cairo_lib=\n")
    f.write("python_include=\n")
    f.write("python_lib=\n")
    f.write("pango_include=\n")
    f.write("pango_lib=\n")
    f.write("glib_include=\n")
    f.write("glib_lib=\n")
    f.write("glib_config_include=\n")
    if platform == 'msw':
        f.write("[nsis]\n")
        f.write("makensis=\n")
    f.write("[other]\n")
    if platform != 'msw':
        f.write('compiler=gcc\n')
    f.write("parallell_compiles=0\n")
    f.write("etags_folder=\n")
    print('Config file "build.cfg" created.\nYou must update the file with correct paths.')


def read_config(platform):
    bo = bs.BuildOptions()
    bo.platform = platform
    config = configparser.RawConfigParser()
    config.read( 'build.cfg')
    wx_root = config.get('folders', 'wx_root')
    wx_vc_lib = join_path(wx_root, "lib", "vc_lib")

    cairo_include = config.get('folders', 'cairo_include')
    cairo_lib = config.get('folders', 'cairo_lib')

    pango_include = config.get('folders', 'pango_include')
    pango_lib = config.get('folders', 'pango_lib')

    python_include = config.get('folders', 'python_include')
    python_lib = config.get('folders', 'python_lib')

    glib_include = config.get('folders', 'glib_include')
    glib_lib = config.get('folders', 'glib_lib')
    glib_config_include = config.get('folders', 'glib_config_include')
    bo.parallell_compiles = int(config.get('other', 'parallell_compiles'))

    bo.extra_resource_root = wx_root
    if bo.platform == 'msw':
        bo.makensis_exe = config.get('nsis', 'makensis')

    if bo.platform == 'linux':
        compiler = config.get('other', 'compiler')
        if compiler is None:
            print("Error: Compiler not specified in build.cfg.")
            print("Expected compiler=clang or compiler=gcc under [other].")
            exit(1)
        elif compiler not in ('gcc', 'clang', 'iwyu'):
            print('Error: Unsupported compiler specified in build.cfg: "%s"'
                  % compiler)
            print('Expected "clang", "gcc" or "iwyu"')
            exit(1)
        bo.compiler = compiler
    elif bo.platform == 'msw':
        bo.compiler = 'msvc'

    if (wx_root == "" or
        python_lib == "" or
        python_include == "" or
        cairo_include == "" or
        pango_include == ""):

        print("Error: Incorrect paths in build.cfg")
        exit(1)

    if cairo_lib == "" and not platform.startswith("linux"):
        print("Error: Incorrect paths in build.cfg")
        exit(1)

    bo.lib_paths = [
        cairo_lib,
        pango_lib,
        python_lib,
        glib_lib]

    bo.lib_paths = [l for l in bo.lib_paths if len(l) != 0]

    if bo.platform == "msw":
        bo.lib_paths.append(join_path(wx_root, 'lib', 'vc_lib'))


    bo.project_root = faint_info.FAINT_ROOT
    bo.system_include_folders = [
        join_path( wx_vc_lib, "mswu" ),
        join_path( wx_root, "include" ),
        python_include,
        cairo_include,
        pango_include,
        glib_include,
        glib_config_include,
    ]

    bo.include_folders = [bo.project_root]

    bo.wx_root = wx_root
    return bo


def read_build_options(platform):
    if not os.path.exists("build.cfg"):
        recreate_config(platform)
        exit(1)

    return read_config(platform)


def test_extra_objs(bo):
    if bo.debug_compile:
        obj_root = join_path(os.getcwd(), "objs-debug")
    else:
        obj_root = join_path(os.getcwd(), "objs-release")

    return [join_path(obj_root, strip_ext(item)) for item in
            os.listdir(join_path(os.getcwd(), obj_root))
            if (item.endswith('.obj') or item.endswith('.o')) and
            not item.startswith('app.')]


def test_source_files(platform, bo, folder):
    test_source_folder = join_path(bo.project_root, folder)
    test_root = join_path(bo.project_root, "tests")
    test_files = []
    for folder in (test_source_folder,
                   join_path(test_source_folder, 'gen'),
                   join_path(test_root, "test-util")):
        test_files.extend([join_path(folder, f)
                           for f in list_cpp(folder)])
    return test_files


def build(caption, platform, cmdline, obj_folder_prefix, out_name, precompile_steps, source_files, source_folders, extra_objs, msw_subsystem, forced_include_func):
    print(caption)
    print("--------------------")
    bo = read_build_options(platform)
    bo.obj_root_release = join_path(os.getcwd(), "%s-release" % obj_folder_prefix)
    bo.obj_root_debug = join_path(os.getcwd(), "%s-debug" % obj_folder_prefix)
    bo.extra_objs = extra_objs(bo)
    bo.out_name_release = out_name
    bo.out_name_debug = out_name + "d"
    opts, args = cmdline
    bo.debug_compile = opts.debug
    precompile_steps(bo)
    bo.source_files = source_files(platform, bo)
    bo.source_folders = source_folders(platform, False)
    bo.forced_include = forced_include_func(bo)
    bo.msw_subsystem = msw_subsystem
    return bs.build(bo, cmdline)


def exit_on_error(function, args, blank_line=True):
    if blank_line:
        print()
    return_code = function(*args)
    if return_code != 0:
        exit(return_code)


def run_unit_tests(platform, cmdline):
    extension = ".exe" if platform == "msw" else ""
    test_root = join_path(faint_info.FAINT_ROOT, "tests")
    cmd = join_path(test_root, "run-unit-tests" + extension) + " --silent"
    result = subprocess.call(cmd,
                             shell=True,
                             cwd=test_root)
    if result == 0:
        print("* Tests OK")
    else:
        print("* Unit tests failed!")
    return result


def build_faint(platform, cmdline):
    target = faint_info.target_faint
    def faint_source_files(platform, project_root):
        src_folders = faint_info.get_src_folders(platform)
        src_folders = [join_path(project_root, folder)
                for folder in src_folders ]
        src_folders.append(project_root)

        files = []
        for folder in src_folders:
            files.extend([join_path(folder, f)
                         for f in list_cpp(folder)])
        return files

    def precompile_steps(bo):
        # Generate setting-handling code based on set_and_get.py
        gencpp.run("../python/generate")

        if not os.path.exists("../help/source/generated"):
            os.mkdir("../help/source/generated")

        bs.gen_method_def.generate_headers(faint_info.HEADERS_TO_GENERATE,
                                           faint_info.GENERATED_METHOD_DEF_PATH,
                                           faint_info.GENERATED_HELP_PATH)

        bs.gen_resource.run(bo.project_root)

        bs.gen_text_expressions.generate(
            hh_path=join_path(bo.project_root,
                              "generated", "text-expression-constants.hh"),
            help_path=join_path(faint_info.GENERATED_HELP_PATH,
                                "text-expressions.txt"))
        # HTML help
        bs.gen_help.run()

    return build(
        "Faint",
        platform,
        cmdline,
        "objs",
        "faint",
        precompile_steps,
        lambda platform, bo: faint_source_files(platform, bo.project_root),
        lambda platform, test: faint_info.get_src_folders(platform, test),
        lambda bo: [],
        "windows",
        lambda bo: join_path(bo.project_root, "util", "msw_warn.hh"))


def build_benchmarks(platform, cmdline):
    target = faint_info.target_benchmark
    def precompile_steps(bo):
        bench_root = join_path(bo.project_root, target.source_folder)
        test_root = join_path(bo.project_root, "tests")
        gen_runner.gen_bench_runner(root_dir=bench_root,
            out_file=join_path(bench_root, 'gen', 'bench-runner.cpp'))
        bo.create_build_info = False

    return build(
        "Benchmarks",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        lambda platform, bo: test_source_files(platform, bo, target.source_folder),
        lambda platform, test: [],
        test_extra_objs,
        "console",
        lambda bo: join_path(bo.project_root, "util", "msw_warn.hh"))


def build_unit_tests(platform, cmdline):
    target = faint_info.target_unit_test

    def precompile_steps(bo):
        tests_root = join_path(bo.project_root, target.source_folder)
        test_root = join_path(bo.project_root, "tests")
        gen_runner.gen_test_runner(root_dir=tests_root,
            out_file=join_path(tests_root, 'gen', 'test-runner.cpp'))
        bo.create_build_info = False

    return build(
        "Unit tests",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        lambda platform, bo: test_source_files(platform, bo, target.source_folder),
        lambda platform, test: [],
        test_extra_objs,
        "console",
        lambda bo: join_path(bo.project_root, "util", "msw_warn.hh"))


def build_image_tests(platform, cmdline):
    target = faint_info.target_image_test

    def precompile_steps(bo):
        tests_root = join_path(bo.project_root, target.source_folder)
        gen_runner.gen_image_runner(root_dir=tests_root,
            out_file=join_path(tests_root, 'gen', 'image-runner.cpp'))
        bo.create_build_info = False

    return build(
        "Image tests",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        lambda platform, bo: test_source_files(platform, bo, target.source_folder),
        lambda platform, test: [],
        test_extra_objs,
        "console",
        lambda bo: join_path(bo.project_root, "util", "msw_warn.hh"))


def build_gui_tests(platform, cmdline):
    target = faint_info.target_gui_test
    def precompile_steps(bo):
        bo.create_build_info = False

    def test_source_files(platform, bo, folder):
        test_source_folder = join_path(bo.project_root, folder)
        test_root = join_path(bo.project_root, "tests")
        test_files = []
        for folder in (test_source_folder,
                       join_path(test_root, "test-util")):
            test_files.extend([join_path(folder, f)
                               for f in list_cpp(folder)])
        return test_files


    return build(
        "GUI-tests",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        lambda platform, bo: test_source_files(platform, bo, target.source_folder),
        lambda platform, test: [],
        test_extra_objs,
        "windows",
        lambda bo: join_path(bo.project_root, "util", "msw_warn.hh"))


if __name__ == '__main__':
    platform = ("linux" if sys.platform.startswith('linux') else "msw")
    cmdline = bs.parse_command_line()
    opts, args = cmdline

    exit_on_error(build_faint, (platform, cmdline), blank_line=False)

    if opts.debug:
        print("Fixme: Not building tests in debug.")
    else:
        exit_on_error(build_unit_tests, (platform, cmdline))
        exit_on_error(build_image_tests, (platform, cmdline))
        exit_on_error(build_benchmarks, (platform, cmdline))
        exit_on_error(build_gui_tests, (platform, cmdline))
        exit_on_error(run_unit_tests, (platform, cmdline))

    exit(0)
