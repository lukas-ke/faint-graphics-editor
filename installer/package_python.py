#!/usr/bin/env python3

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

import os
import shutil
import zipfile

folders = [
    "collections", # "Container datatypes"
    "concurrent", # Parent for futures
    "concurrent/futures", # Launching parallel tasks
    "ctypes", # A foreign function library for Python.
    #ctypes/macholib", used to edit Mach-O headers (executable format on Mac OS)
    # "curses", # An interface to the curses library, providing portable terminal handling.
    # "dbm", # Interfaces to Unix "databases"
    # "distutils", # Building and installing Python modules
    # "distutils/commands", # Standard distutils commands
    # "distutils/tests", # Test suite for distutils
    # "email", # An email and MIME handling package
    # "email/mime", # MIME type handling, I guess
    "encodings", # Internationalized Domain Names implementation
    # "hotshot", # High performance logging profiler, mostly written in C.
    "html", # HyperText Markup Language support
    # "idlelib",
    "importlib", # An implementation of import
    "json", # Encode and decode the JSON format.
    # "lib2to3", # the 2to3 library (convert Python 2.# to 3)
    # "logging", # Logging facility for Python
    # "msilib", # Read and write Microsoft Installer files
    "multiprocessing", # Process-based parallelism
    "multiprocessing/dummy", # Wrapper around the threading module
    # "pydoc_data", # Documentation generator and online help system
    # "site-packages", Extensions
    "sqlite3", # DB-API 2.0 interface for SQLite databases
    "sqlite3/test", # Presumably sqlite3 unit tests
    # "test", # Regression tests package containing the testing suite for Python.
    # "tkinter", # Tk gui toolkit interfaces
    # "turtledemo", # Turtle graphics toy
    # "unittest", Unit testing framework
    "urllib", # URL handling modules
    #"venv", # Creation of virtual Python environments
    #"venv/scripts", # Creation of virtual Python environments
    #"wsgiref", # Web Server Gateway Interface Utilities and Reference Implementation
    "xml", # XML Processing Modules
    "xml/dom", # The Document Object Model API
    "xml/etree", # The ElementTree XML API
    "xml/parsers", # Expat
    "xml/sax", # SAX parser
    #"xmlrpc", # XMLRPC server and client modules
    ]

dlls = [
    "_bz2.pyd",
    "_ctypes.pyd",
    # "_ctypes_test.pyd",
    "_decimal.pyd",
    "_elementtree.pyd",
    "_hashlib.pyd",
    "_lzma.pyd",
    #"_msi.pyd",
    "_multiprocessing.pyd",
    "_socket.pyd",
    "_sqlite3.pyd",
    "_ssl.pyd",
    #"_testbuffer.pyd",
    #"_testcapi.pyd",
    # "_tkinter.pyd"
    # "py.ico",
    # "pyc.ico",
    "pyexpat.pyd",
    #"python3.dll",
    "select.pyd",
    "sqlite3.dll",
    # "tcl85.dll",
    # "tclpip85.dll",
    # "tk85.dll",
    "unicodedata.pyd",
    # "winsound.pyd",
]

def zipit():
    if not os.path.exists("installdep"):
        os.mkdir("installdep")
        os.mkdir("installdep/python")
        os.mkdir("installdep/SysWOW64")
    zf = zipfile.ZipFile('installdep/python/python33.zip', mode='w')
    for file in [file for file in os.listdir("pythonbundle") if file.endswith(".py")]:
        zf.write(os.path.join("pythonbundle", file), file)

    for folder in folders:
        srcFolder = os.path.join("pythonbundle", folder)
        zf.write(srcFolder, folder )
        for file in [ file for file in os.listdir(srcFolder) if file.endswith(".py")]:
            zf.write( os.path.join(srcFolder, file), os.path.join(folder, file))

def create_python_bundle(pythonRoot):
    pythonLib = os.path.join(pythonRoot, "lib")
    for file in [ os.path.join(pythonLib, file) for file in os.listdir(pythonLib) if file.endswith(".py")]:
        print("Warning: Not compiling: ", file)
        #compiler.compileFile( file )
    for file in [ os.path.join(pythonLib, file) for file in os.listdir(pythonLib) if file.endswith(".py")]:
        shutil.copy(file, "pythonbundle")

    for folder in folders:
        srcFolder = os.path.join(pythonLib, folder)
        dstFolder = os.path.join("pythonbundle", folder)
        if not os.path.exists(dstFolder):
            os.mkdir(dstFolder)

        for file in [os.path.join(srcFolder, file) for file in os.listdir(srcFolder) if file.endswith(".py")]:
            print("Warning: Not compiling: ", file)
            #print("Compiling: ", file)
            #compiler.compileFile( file )
        for file in [ os.path.join(srcFolder, file) for file in os.listdir(srcFolder) if file.endswith(".py")]:
            shutil.copy(file, dstFolder)

def copy_pyd( pythonRoot, dest ):
    for file in dlls:
        shutil.copy(os.path.join(pythonRoot, "DLLs", file), dest)

if not os.path.exists("pythonbundle"):
    os.mkdir("pythonbundle")

if __name__ == '__main__':
    import sys
    pythonRoot = sys.argv[1]
    create_python_bundle(pythonRoot)
    zipit()
    copy_pyd(pythonRoot, "installdep\\python")
