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
import sys
import faint_info

build_dir = os.path.split(os.path.realpath(__file__))[0]
os.chdir(build_dir) # Fixme: Don't change dir, use absolute paths.
root_dir = os.path.split(build_dir)[0]

sys.path.append(os.path.join(root_dir, "build-sys"))
import build_sys.gen_method_def as gen_method_def

def matches_any_ext(filename, extensions):
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
               ( 'settings',),
               ('.hh', '.cpp', '.txt'))

    clean_path(os.path.join(faintDir, 'generated', 'python'),
               ( 'method-def',),
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

    gen_method_def.clean(faint_info.HEADERS_TO_GENERATE,
       faint_info.GENERATED_HELP_PATH)


def clean_help(faintDir):
    helpDir = os.path.join(faintDir, "help")
    for file in os.listdir(helpDir):
        if file.endswith(".html") or file.endswith(".dat"):
            os.remove(os.path.join(helpDir, file))

def clean_obj(faintDir):
    jp = os.path.join
    dirs = [
        jp(faintDir, "build", "objs-bench-release"),
        jp(faintDir, "build", "objs-debug"),
        jp(faintDir, "build", "objs-image-test-debug"),
        jp(faintDir, "build", "objs-image-test-release"),
        jp(faintDir, "build", "objs-release"),
        jp(faintDir, "build", "objs-test-debug"), # Fixme: Rename to objs-unit-test
        jp(faintDir, "build", "objs-test-release"),
        jp(faintDir, "build", "objs-test-debug"),
        jp(faintDir, "build", "objs-gui-test-release"),
        jp(faintDir, "build", "objs-gui-test-debug"),
    ]

    for objDir in dirs:
        if os.path.exists(objDir):
            for file in os.listdir(objDir):
                if file.endswith('.o') or file.endswith('.obj') or file.endswith(".res") or file.endswith(".pdb"):
                    os.remove(os.path.join(objDir, file))

            if len(os.listdir(objDir)) == 0:
                os.rmdir(objDir)

    for f in (os.path.join(faintDir, "build", "out.txt"),
              os.path.join(faintDir, "build", "err.txt")):
        if os.path.exists(f):
            os.remove(f)


def clean_exe(faintDir):
    jp = os.path.join
    # Fixme: Windows-centric
    # Fixme: Derive from faint_info.target_*
    executables = [f for f in (
        jp(faintDir, "faint.exe"),
        jp(faintDir, "faintd.exe"),
        jp(faintDir, "tests", "run-benchmarks.exe"),
        jp(faintDir, "tests", "run-image-tests.exe"),
        jp(faintDir, "tests", "run-unit-tests.exe"),
        jp(faintDir, "tests", "run-gui-tests.exe"),
    ) if os.path.exists(f)]

    for f in executables:
        os.remove(f)


known_args=(("all",
             "everything",
             (clean_help,
              clean_generated,
              clean_obj,
              clean_exe)),
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
