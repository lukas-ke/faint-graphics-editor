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

import configparser
import os
import subprocess
import sys

import faint_info

join_path = os.path.join

build_dir = os.path.split(os.path.realpath(__file__))[0]
os.chdir(build_dir)  # Fixme: Don't change dir, use absolute paths.

root_dir = os.path.split(build_dir)[0]
sys.path.append(join_path(root_dir, "build-sys/"))
sys.path.append(join_path(root_dir, "test-sys/"))
import build_sys as bs  # noqa: E402
from build_sys.util import list_cpp, strip_ext  # noqa: E402
from build_sys.util.scoped import working_dir, no_output  # noqa: E402
from test_sys import gen_runner  # noqa: E402
import gencpp  # noqa: E402


def recreate_config(platform):
    with open("build.cfg", 'w') as f:
        f = open("build.cfg", 'w')
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
        f.write("pnglib_include=\n")
        if platform == 'msw':
            f.write("[nsis]\n")
            f.write("makensis=\n")
        f.write("[other]\n")
        if platform != 'msw':
            f.write('compiler=gcc\n')
        f.write("parallell_compiles=0\n")
        f.write("etags_folder=\n")

    print('Config file "build.cfg" created.\n'
          'You must update the file with correct paths.')


def read_config(platform):
    def check_folder(name, folder, expected_content):
        """Verify that this folder (from an entry in the build.cfg) contains
        some expected file.

        """

        full_path = os.path.expanduser(os.path.join(folder, expected_content))

        if not os.path.exists(full_path):
            print(f'Error in build.cfg:\n {name}: {expected_content} not found in \n {folder}')  # noqa: E501
            print(full_path)
            exit(1)

    bo = bs.BuildOptions()
    bo.platform = platform
    config = configparser.RawConfigParser()
    config.read('build.cfg')
    try:
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

        pnglib_include = config.get('folders', 'pnglib_include')
    except configparser.NoOptionError as e:
        print("Error in build.cfg:", e)
        exit(1)

    # Verify that the specified paths contain expected includes or folders
    check_folder("wx_root", wx_root, "include/wx")
    check_folder("cairo_include", cairo_include, "cairo.h")
    check_folder("python_include", python_include, "Python.h")
    check_folder("pango_include", pango_include, "pango/pango.h")
    check_folder("pnglib_include", pnglib_include, "png.h")
    check_folder("glib_include", glib_include, "glib.h")
    check_folder("glib_config_include", glib_config_include, "glibconfig.h")

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
            print(f'Error: Unsupported compiler specified in build.cfg: "{compiler}"')  # noqa: E501
            print('Expected "clang", "gcc" or "iwyu"')
            exit(1)
        bo.compiler = compiler
    elif bo.platform == 'msw':
        bo.compiler = 'msvc'

    required_path_empty = (wx_root == "" or
                           python_lib == "" or
                           python_include == "" or
                           cairo_include == "" or
                           pango_include == "" or
                           pnglib_include == "")
    if required_path_empty:
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
        join_path(wx_vc_lib, "mswu"),
        join_path(wx_root, "include"),
        python_include,
        cairo_include,
        pango_include,
        glib_include,
        glib_config_include,
        pnglib_include
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
    def excluded(obj):
        return (obj.startswith('app.')
                or obj.startswith('py-initialize-ifaint.'))

    obj_root = join_path(os.getcwd(),
                         faint_info.target.faint.objs_folder_prefix)
    obj_root = obj_root + ("-debug" if bo.debug_compile else "-release")

    return [join_path(obj_root, strip_ext(item)) for item in
            os.listdir(join_path(os.getcwd(), obj_root))
            if (item.endswith('.obj') or item.endswith('.o')) and
            not excluded(item)]


def get_test_source_files(bo, folder):
    test_source_folder = join_path(bo.project_root, folder)
    test_root = join_path(bo.project_root, "tests")
    test_files = []
    for folder in (test_source_folder,
                   join_path(test_source_folder, 'gen'),
                   join_path(test_root, "test-util")):
        test_files.extend([join_path(folder, f)
                           for f in list_cpp(folder)])
    return test_files


def no_source_folders_f(*args, **kwArgs):
    return []


def build(caption,
          platform,
          cmdline,
          obj_folder_prefix,
          out_name,
          precompile_steps,
          source_files,
          source_folders,
          extra_objs,
          msw_subsystem,
          forced_include_func):
    print(caption)
    print("--------------------")
    bo = read_build_options(platform)
    bo.obj_root_release = join_path(
        os.getcwd(),
        f"{obj_folder_prefix}-release")
    bo.obj_root_debug = join_path(
        os.getcwd(),
        f"{obj_folder_prefix}-debug")
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
        print("* C++ Unit tests OK")
    else:
        print("* C++ Unit tests failed!")
    return result


def run_py_tests(platform, cmdline):
    sys.path.append(faint_info.FAINT_TESTS_ROOT)
    import run_py_tests as py_tests

    with no_output(), working_dir(faint_info.FAINT_TESTS_ROOT):
        ok = py_tests.run_tests()
    if ok:
        print('* Python Unit tests OK')
        return 0
    else:
        print("* Error: Python Unit tests failed!")
        return 1


def forced_include_func(bo):
    return join_path(bo.project_root, "util", "msw_warn.hh")


def build_faint(platform, cmdline):

    def precompile_steps(bo):
        # Generate setting-handling code based on set_and_get.py
        gencpp.run("../python/generate")

        if not os.path.exists("../help/source/generated"):
            os.mkdir("../help/source/generated")

        bs.gen_method_def.generate_headers(
            faint_info.HEADERS_TO_GENERATE,
            faint_info.GENERATED_METHOD_DEF_PATH,
            faint_info.GENERATED_HELP_PATH)

        bs.gen_resource.run(bo.project_root)

        bs.gen_text_expressions.generate(
            hh_path=join_path(
                bo.project_root,
                "generated", "text-expression-constants.hh"),
            help_path=join_path(
                faint_info.GENERATED_HELP_PATH,
                "text-expressions.txt"))

        # HTML help
        bs.gen_help.run()

    def get_faint_src_files(platform, bo):
        src_folders = faint_info.get_src_folders(platform)
        src_folders = [join_path(bo.project_root, folder)
                       for folder in src_folders]
        src_folders.append(bo.project_root)

        files = []
        for folder in src_folders:
            files.extend([join_path(folder, f)
                         for f in list_cpp(folder)])
        return files

    def get_faint_extra_objs(bo):
        return []

    return build(
        "Faint",
        platform,
        cmdline,
        "objs",
        "faint",
        precompile_steps,
        get_faint_src_files,
        faint_info.get_src_folders,
        get_faint_extra_objs,
        "windows",
        forced_include_func)


def build_benchmarks(platform, cmdline):
    target = faint_info.target.benchmark

    def precompile_steps(bo):
        bench_root = join_path(bo.project_root, target.source_folder)
        gen_runner.gen_bench_runner(
            root_dir=bench_root,
            out_file=join_path(bench_root, 'gen', 'bench-runner.cpp'))
        bo.create_build_info = False

    def get_benchmark_source_files(platform_, bo):
        return get_test_source_files(bo, target.source_folder)

    return build(
        "Benchmarks",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        get_benchmark_source_files,
        no_source_folders_f,
        test_extra_objs,
        "console",
        forced_include_func)


def build_unit_tests(platform, cmdline):
    target = faint_info.target.unit_test

    def precompile_steps(bo):
        tests_root = join_path(bo.project_root, target.source_folder)
        gen_runner.gen_test_runner(
            root_dir=tests_root,
            out_file=join_path(tests_root, 'gen', 'test-runner.cpp'))
        bo.create_build_info = False

    def get_unit_test_source_files(platform, bo):
        return get_test_source_files(bo, target.source_folder)

    return build(
        "Unit tests",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        get_unit_test_source_files,
        no_source_folders_f,
        test_extra_objs,
        "console",
        forced_include_func)


def build_image_tests(platform, cmdline):
    target = faint_info.target.image_test

    def precompile_steps(bo):
        tests_root = join_path(bo.project_root, target.source_folder)
        gen_runner.gen_image_runner(
            root_dir=tests_root,
            out_file=join_path(tests_root, 'gen', 'image-runner.cpp'))
        bo.create_build_info = False

    def get_image_test_source_files(platform, bo):
        return get_test_source_files(bo, target.source_folder)

    return build(
        "Image tests",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.executable,
        precompile_steps,
        get_image_test_source_files,
        no_source_folders_f,
        test_extra_objs,
        "console",
        forced_include_func)


def build_gui_tests(platform, cmdline):
    target = faint_info.target.gui_test

    def precompile_steps(bo):
        bo.create_build_info = False

    def get_gui_test_source_files(platform, bo):
        test_source_folder = join_path(bo.project_root, target.source_folder)
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
        get_gui_test_source_files,
        no_source_folders_f,
        test_extra_objs,
        "windows",
        forced_include_func)


def build_python_extension(platform, cmdline):
    def precompile_steps(bo):
        bo.create_build_info = False
        bo.target_type = bo.Target.shared_python_library
        if not os.path.exists("../ext/out"):
            os.mkdir("../ext/out")
    target = faint_info.target.python_extension

    def extension_source_files(platform, bo):
        src_folder = join_path(bo.project_root, target.source_folder)
        return [join_path(src_folder, f) for f in list_cpp(src_folder)]

    result = build(
        "Python extension",
        platform,
        cmdline,
        target.objs_folder_prefix,
        target.out_lib,
        precompile_steps,
        extension_source_files,
        no_source_folders_f,
        test_extra_objs,
        "console",
        forced_include_func)

    return result


if __name__ == '__main__':
    platform = ("linux" if sys.platform.startswith('linux') else "msw")
    cmdline = bs.parse_command_line()
    opts, args = cmdline

    exit_on_error(build_faint, (platform, cmdline), blank_line=False)

    if platform == 'msw':  # Py-extension build not implemented for Linux yet.
        exit_on_error(build_python_extension, (platform, cmdline))

    if opts.debug:
        print("Fixme: Not building tests in debug.")
    else:
        exit_on_error(build_unit_tests, (platform, cmdline))
        exit_on_error(build_image_tests, (platform, cmdline))
        exit_on_error(build_benchmarks, (platform, cmdline))
        exit_on_error(build_gui_tests, (platform, cmdline))

        exit_on_error(run_unit_tests, (platform, cmdline))

    if platform == 'msw':
        exit_on_error(run_py_tests, (platform, cmdline))

    if opts.version != bs.unknown_version_str and platform == 'msw':
        bo = read_build_options(platform)

        bs.build_installer(opts.version, bo.makensis_exe)

    exit(0)
