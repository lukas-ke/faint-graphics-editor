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

class BuildOptions:
    """Options for the build."""
    def __init__(self):
        # Libs to link
        #self.libs = None

        # Folders to add to lib search
        self.lib_paths = None

        # Folders, within project, to add to include path
        self.include_folders = None

        # External includes, to be added as system includes (if
        # possible)
        self.system_include_folders = None

        # Cpp-files to compile
        self.source_files = None

        # Source folders to search
        self.source_folders = None

        # gcc, clang or msvc
        self.compiler = None

        # msw or linux
        self.platform = None

        # Path to nsis-executable for building installers on Windows.
        self.makensis_exe = None

        # Folder for compiled object files (release).
        self.obj_root_release = None

        # Folder for compiled object files (debug).
        self.obj_root_debug = None

        # Name for the built application, when release (without extension)
        self.out_name_release = None

        # Name for the built application, when debug (without extension)
        self.out_name_debug = None

        # Root folder for the project
        self.project_root = None

        self.parallell_compiles = None
        self.extra_resource_root = None
        self.wx_root = None
        self.test_dir = None

        self.check_deps = True
        self.create_build_info = True

        self.extra_objs = []

        self.forced_include = None
        self.msw_subsystem = None
        self.__setattr__ = self.setattr
        self.optional = ["makensis_exe",
                         "extra_resource_root",
                         "test_dir",
                         "forced_include",
                         "msw_subsystem"]

        self.debug_compile = None

        # Time various build steps?
        self.timed = False

    def get_obj_root(self):
        assert(self.debug_compile is not None)
        root = (self.obj_root_debug if self.debug_compile
                else self.obj_root_release)
        assert(root is not None)
        return root

    def get_out_name(self):
        assert(self.debug_compile is not None)
        return (self.out_name_debug if self.debug_compile
                else self.out_name_release)

    def get_out_path(self):
        out_path = os.path.join(self.project_root, self.get_out_name())
        if self.platform == 'msw':
            out_path += '.exe'
        return out_path

    def set_debug_compile(self, debug_compile):
        self.debug_compile = debug_compile

    def setattr(self, name, value):
        assert(name in self.__dict__)
        self.__dict__[name] = value

    def verify(self):
        vars = [item for item in self.__dict__ if not item.startswith("_")]
        for var in vars:
            if (self.__dict__[var] is None and var not in self.optional):
                print('%s not initialized.' % var)
                exit(1)

        if self.msw_subsystem is None and self.platform == "msw":
            print("msw_subsystem not initialized.")
            exit(1)

        if self.wx_root is None and self.platform == "linux":
            print("wx_root not initialized.")
            exit(1)
