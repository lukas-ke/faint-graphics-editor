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

"""Script for cleaning Faint output."""

import os
import sys
import faint_info
import shutil

build_dir = os.path.split(os.path.realpath(__file__))[0]
os.chdir(build_dir)  # Fixme: Don't change dir, use absolute paths.
root_dir = os.path.split(build_dir)[0]

sys.path.append(os.path.join(root_dir, "build-sys"))
import build_sys.gen_method_def as gen_method_def  # noqa: E402


def existing(paths):
    return [p for p in paths if os.path.exists(p)]


def remove_dir_if_empty(d):
    if len(os.listdir(d)) == 0:
        os.rmdir(d)


def matches_any_ext(filename, extensions):
    # Fixme: Use any
    for ext in extensions:
        if filename.endswith(ext):
            return True
    return False


def erase_matching_files(path, extensions):
    for filename in os.listdir(path):
        if matches_any_ext(filename, extensions):
            os.remove(os.path.join(path, filename))


def clean_path(base, levels, extensions):
    assert levels.__class__ == tuple
    while len(levels) > 0:
        path = os.path.join(base, *levels)
        levels = levels[:-1]
        if os.path.exists(path):
            erase_matching_files(path, extensions)
            os.rmdir(path)


def clean_generated(faintDir):
    clean_path(os.path.join(faintDir, 'generated', 'python'),
               ('settings',),
               ('.hh', '.cpp', '.txt'))

    clean_path(os.path.join(faintDir, 'generated', 'python'),
               ('method-def',),
               ('.hh', '.cpp', '.txt'))

    clean_path(os.path.join(faintDir, 'generated'),
               ('python',),
               ('.hh', '.cpp', '.txt'))

    clean_path(os.path.join(faintDir, 'generated'),
               ('resource',),
               ('.hh', '.cpp', '.txt'))

    clean_path(faintDir, ('generated',),
               ('.hh', '.cpp', '.txt'))

    clean_path(os.path.join(faintDir, 'tests', 'unit-tests'), ('gen',),
               ('.hh', '.cpp'))

    clean_path(os.path.join(faintDir, 'tests', 'benchmarks'), ('gen',),
               ('.hh', '.cpp'))

    clean_path(os.path.join(faintDir, 'tests', 'image-tests'), ('gen',),
               ('.hh', '.cpp'))

    clean_path(os.path.join(faintDir, 'help', 'source'), ('generated',),
               ('.txt', '.html'))

    generated_makefile = os.path.join(faintDir, "build/generated_makefile")
    if os.path.exists(generated_makefile):
        os.remove(generated_makefile)

    gen_method_def.clean(
        faint_info.HEADERS_TO_GENERATE,
        faint_info.GENERATED_HELP_PATH)


def clean_test_output(faintDir):
    test_out_dir = os.path.join(faintDir, 'tests', 'out')
    if os.path.exists(test_out_dir):
        shutil.rmtree(test_out_dir)


def clean_help(faintDir):
    helpDir = os.path.join(faintDir, "help")
    for file in os.listdir(helpDir):
        if file.endswith(".html") or file.endswith(".dat"):
            os.remove(os.path.join(helpDir, file))


def get_obj_folders(faintDir):
    jp = os.path.join
    bd = jp(faintDir, "build")
    prefixes = [t.objs_folder_prefix for t in faint_info.TARGETS]
    prefixes_release = [p + "-release" for p in prefixes]
    prefixes_debug = [p + "-debug" for p in prefixes]
    return [jp(bd, p) for p in (prefixes_release + prefixes_debug)]


def clean_obj(faintDir):
    jp = os.path.join

    dirs = get_obj_folders(faintDir)

    def should_remove(f):
        cleaned_exts = [
            '.o',
            '.obj',
            '.res',
            '.pdb',
        ]
        return any([f.endswith(ext) for ext in cleaned_exts])

    def maybe_remove(f):
        if should_remove(f):
            os.remove(f)

    for obj_dir in existing(dirs):
        for f in os.listdir(obj_dir):
            maybe_remove(jp(obj_dir, f))
        remove_dir_if_empty(obj_dir)

    bd = jp(faintDir, "build")
    for f in existing((jp(bd, "out.txt"), jp(bd, "err.txt"))):
        os.remove(f)


def clean_exe(faintDir):
    jp = os.path.join
    # Fixme: Windows-centric

    t = faint_info.target
    executables = existing([
        jp(faintDir, t.faint.executable + ".exe"),
        jp(faintDir, t.faint.executable + "d.exe"),
        jp(faintDir, t.benchmark.executable + ".exe"),
        jp(faintDir, t.gui_test.executable + ".exe"),
        jp(faintDir, t.image_test.executable + ".exe"),
        jp(faintDir, t.unit_test.executable + ".exe"),
        jp(faintDir, t.python_extension.out_lib + ".pyd"),
        jp(faintDir, t.python_extension.out_lib + ".pyd.manifest")])

    for f in executables:
        os.remove(f)


known_args = (("all",
               "everything",
               (clean_help,
                clean_generated,
                clean_obj,
                clean_exe,
                clean_test_output)),
              ("exe", "Faint executables", (clean_exe,)),
              ("html", "the help", (clean_help,)),
              ("gen", "generated code", (clean_generated,)),
              ("obj", "built object files", (clean_obj,)))


def main():
    if len(sys.argv) == 1:
        print("Arguments:")
        print(" " + "\n ".join([arg[0] + "\t" + arg[1] for arg in known_args]))
        exit(1)

    argnames = [ka[0] for ka in known_args]
    for arg in sys.argv[1:]:
        if arg not in argnames:
            print("Unknown argument: " + arg)
            exit(1)

    rootDir = os.path.abspath("../")
    path_check = ("app", "build", "tools", "objects", "util")
    rootFiles = os.listdir(rootDir)
    for file in path_check:
        if file not in rootFiles:
            print("Error: Didn't find expected folders in %s" % rootDir)
            exit(1)

    for arg in sys.argv:
        for ka in known_args:
            if arg == ka[0]:
                for func in ka[2]:
                    func(rootDir)


if __name__ == '__main__':
    main()
