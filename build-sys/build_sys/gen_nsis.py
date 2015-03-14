#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2012 Lukas Kemmer
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

def match_ext( extensions, filename ):
    if extensions is None:
        print("extensions is None", filename)
        return True
    elif extensions.__class__ ==  "".__class__:
        print("extensions is str: ", filename)
        return filename.endswith(extensions)
    else:
        for ext in extensions:
            if filename.endswith(ext):
                return True

    return False

def list_join( path, prefix, extensions=None ):
    prefix = prefix.replace("/","\\")
    return [os.path.join(prefix, filename) for filename in os.listdir(path) if match_ext(extensions, filename)]

def find_files():
    targetDir = "..\\"
    sourceDir = "..\\"
    installDepDir = os.path.join(sourceDir, "installer/installdep")
    installDepPython = os.path.join(installDepDir, "python")
    SysWOW64Dir = os.path.join(installDepDir, "SysWOW64")
    paletteDir = os.path.join(sourceDir, "palettes")
    graphicsDir = os.path.join(sourceDir, "graphics")

    helpRootDir = os.path.join(sourceDir, "help")
    helpImageDir = os.path.join(helpRootDir, "images")

    pythonRootDir = os.path.join(sourceDir,"py")
    pythonCoreDir = os.path.join(pythonRootDir, "core")
    pythonPyDir = os.path.join(pythonRootDir, "faint")
    pythonPyExtraDir = os.path.join(pythonRootDir, "faint", "extra")
    pythonPySvgDir = os.path.join(pythonPyDir, "svg")
    pythonPyPdfDir = os.path.join(pythonPyDir, "pdf")

    instDirFiles = list_join( sourceDir, targetDir, ("Faint.exe", ".dll", ".ico"))
    instDirFiles.extend( [os.path.join(sourceDir, "LICENSE"), os.path.join(sourceDir, "NOTICE")] )
    instDirFiles.extend(list_join( installDepPython, installDepPython, (".zip", ".pyd") ))
    instDirFiles.extend(list_join( SysWOW64Dir, SysWOW64Dir, ".dll" ) )

    graphicsFiles = list_join(graphicsDir, graphicsDir, ('.png', '.cur'))

    paletteFiles = list_join(paletteDir, paletteDir, '.txt')

    helpRootFiles = list_join(helpRootDir, helpRootDir, ('.html', '.dat'))
    helpImageFiles = list_join(helpImageDir, helpImageDir, ('.png', '.jpg'))
    pythonPyFiles = list_join(pythonPyDir, pythonPyDir, '.py')
    pythonPyExtraFiles = list_join(pythonPyExtraDir, pythonPyExtraDir, '.py')
    pythonPySvgFiles = list_join(pythonPySvgDir, pythonPySvgDir, '.py')
    pythonPyPdfFiles = list_join(pythonPyPdfDir, pythonPyPdfDir, '.py')
    pythonCoreFiles = list_join(pythonCoreDir, pythonCoreDir, '.py')

    return (("$INSTDIR",instDirFiles),
            ("$INSTDIR\\graphics", graphicsFiles),
            ("$INSTDIR\\palettes", paletteFiles),
            ("$INSTDIR\\help", helpRootFiles),
            ("$INSTDIR\\help\\images", helpImageFiles),
            ("$INSTDIR\\py", []),
            ("$INSTDIR\\py\\core", pythonCoreFiles),
            ("$INSTDIR\\py\\faint", pythonPyFiles),
            ("$INSTDIR\\py\\faint\\extra", pythonPyExtraFiles),
            ("$INSTDIR\\py\\faint\\svg", pythonPySvgFiles),
            ("$INSTDIR\\py\\faint\\pdf", pythonPyPdfFiles))

def files_to_install_string(fileInfo):
    str = ""
    for targetDir, files in fileInfo:
        str += '\nSetOutPath "%s"\n' % targetDir
        for file in files:
            str += 'File "/oname=%s\" "%s"\n' % (file.split("\\")[-1], file)
    return str

def files_to_uninstall_string(fileInfo):
    str = ""
    for targetDir, files in reversed(fileInfo):
        for file in files:
            str += 'Delete "%s\\%s"\n' % (targetDir, file.split("\\")[-1])
        if targetDir == "$INSTDIR":
            str += 'Delete "$INSTDIR\\Uninstall.exe"\n'
        str += 'RMDir "%s"\n\n' % targetDir
    return str

def get_file_size(fileInfo):
    size = 0
    for targetDir, files in fileInfo:
        for file in files:
            size += os.path.getsize(file)
    return size / 1024


def generate_from_template( files, versionStr ):
    template = open('template.nsi', 'r')
    outPath = "out.nsi"
    out = open(outPath, 'w')
    for line in template.readlines():
        if line.find("$$FILES") != -1:
            out.write(files_to_install_string(files))
        elif line.find("$$UNINSTALLFILES") != -1:
            out.write(files_to_uninstall_string(files))
        else:
            if line.find("$$SIZE") != -1:
                out.write(line.replace("$$SIZE", str(get_file_size(files))))
            elif line.find("$$VERSION") != -1:
                out.write(line.replace("$$VERSION", versionStr))
            else:
                out.write(line)
    out.close()
    return outPath


def print_file_list( fileInfo ):
    for targetDir, files in fileInfo:
        print(targetDir)
        for file in files:
            print(" " + file)

def run(versionStr):
    fileInfo = find_files()
    print_file_list( fileInfo )
    outPath = generate_from_template( fileInfo, versionStr )
    return os.path.join(os.getcwd(), outPath)

if __name__ == '__main__':
    versionStr = sys.argv[1]
    run(sys.argv[1])
