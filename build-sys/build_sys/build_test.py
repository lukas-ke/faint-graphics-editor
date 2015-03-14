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

# Fixme: Integrate this with build.py instead

from optparse import OptionParser
from util.util import print_timing
#import configparser
import os
import sys
import genbuildinfo
import gencpp
import genhelp
import gennsis
import genresource
import faint_info

unknown_version_str = "unknown"

def parse_bool( config, group, name ):
    value = config.get(group, name)
    if value.lower() in ['1', 'true', 'yes', 'y']:
        return True
    if value.lower() in ['0', 'false', 'no', 'n']:
        return False
    print("Error: %s should be 1 or 0 in build.cfg" % name)
    exit(1)

def build():
    # Init platform
    if sys.platform.startswith('linux'):
        platform = 'linux'
    else:
        platform = 'msw'


    # Read config
    if not os.path.exists( "build.cfg" ):
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
        exit(1)

    config = configparser.RawConfigParser()
    config.read( 'build.cfg')
    wx_root = config.get( 'folders', 'wx_root' )
    wx_vc_lib = os.path.join( wx_root, "lib", "vc_lib" )

    cairo_include = config.get('folders', 'cairo_include')
    cairo_lib = config.get('folders', 'cairo_lib')

    pango_include = config.get('folders', 'pango_include')
    pango_lib = config.get('folders', 'pango_lib')

    python_include = config.get('folders', 'python_include')
    python_lib = config.get( 'folders', 'python_lib' )

    glib_include = config.get('folders', 'glib_include' )
    glib_lib = config.get('folders', 'glib_lib' )
    glib_config_include = config.get('folders', 'glib_config_include' )
    parallell_compiles = int(config.get('other', 'parallell_compiles'))

    if platform == 'linux':
        compiler = config.get('other','compiler', fallback=None)
        if compiler is None:
            print("Error: Compiler not specified in build.cfg.")
            print("Expected compiler=clang or compiler=gcc under [other].")
            exit(1)
        elif compiler == 'gcc':
            import compile_gcc as compile
        elif compiler == 'clang':
            import compile_clang as compile
        else:
            print('Error: Unsupported compiler specified in build.cfg: "%s"' % compiler)
            print('Expected "clang" or "gcc"')
            exit(1)
    elif platform == 'msw':
        platform = 'msw'
        import compile_msw as compile

    link_test = compile.link_test
    obj_ext = compile.obj_ext
    create_installer = compile.create_installer
    compile = compile.compile

    if platform == 'msw':
        makensis = config.get('nsis', 'makensis') # Oops, not a folder.

    if wx_root == "" or python_lib == "" or python_include == "" or cairo_include == "" or pango_include == "":
        print("Error: Incorrect paths in build.cfg")
        exit(1)

    if cairo_lib == "" and not sys.platform.startswith("linux"):
        print("Error: Incorrect paths in build.cfg")
        exit(1)

    # Read command line options
    optParser = OptionParser()
    optParser.add_option("", "--rebuild",
                      action="store_true", dest="do_rebuild", default=False,
                      help="Rebuild and relink")
    optParser.add_option("", "--version", dest="version", default=unknown_version_str, help="Faint version number")
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
    opts, args = optParser.parse_args()

    def enumerate_obj( folder ):
        return [file for file in os.listdir( folder ) if file.endswith(obj_ext)]

    # Folder for writing/finding object files (.obj, .o)
    objRoot = os.path.join(os.getcwd(), "objs_test")
    objRoot2 = os.path.join(os.getcwd(), "objs")

    cleanBuild = False
    if not os.path.exists( objRoot ):
        os.mkdir( objRoot )
        cleanBuild = True
    elif len(os.listdir(objRoot)) == 0:
        cleanBuild = True

    faintRoot = faint_info.faintRoot

    if not cleanBuild and opts.version != unknown_version_str and not opts.filthy:
        print("Error: Build with --version requires empty output folder (build/objs)")
        exit(1)

    if opts.version != unknown_version_str and not opts.filthy and genbuildinfo.working_copy_modified(faintRoot):
        print("Error: Build with --version requires unmodified working copy.")
        exit(1)

    if opts.version != unknown_version_str and len(makensis) == 0:
        print("Error: Build with --version requires path to nsis in build.cfg")
        exit(1)

    subFolders = faint_info.get_src_folders(platform, test=True)

    folders = [ os.path.join( faintRoot, folder ) for folder in subFolders ]
    folders.append(faintRoot)
    include = [
        os.path.join( wx_vc_lib, "mswu" ),
        os.path.join( wx_root, "include" ),
        python_include,
        cairo_include,
        pango_include,
        glib_include,
        glib_config_include,
    ]

    do_compile = True
    do_link = True
    do_rebuild = opts.do_rebuild

    err = open('err.txt', 'a')
    out = open('out.txt', 'a')

    modified = ()
    to_compile = set()
    depsChanged = set()
    print("* Generating dependencies")
    deps = faint_info.timed(faint_info.get_flat_header_dependencies, faintRoot)

    modified = set()
    depModified = set()
    testFolder = os.path.join(faintRoot, 'test')

    if do_compile:
        for folder in (os.path.join(faintRoot, 'test'), os.path.join(faintRoot, "test/gen")):
            for file in faint_info.enumerate_cpp(folder):
                objFile = os.path.join( objRoot, file.replace( '.cpp', obj_ext ) )
                cppFile = os.path.join( folder, file )
                if not os.path.isfile( objFile ) or do_rebuild or ( os.path.isfile( objFile ) and os.path.getmtime( objFile ) < os.path.getmtime( cppFile ) ):
                    to_compile.add( os.path.join(folder, file ) )
                    modified.add(file)

    if len(to_compile) > 0:
        print("* Compiling")
        print(" ",len(modified), "cpp files modified,", len(depModified), "indirectly.")
        compile( sorted(list(to_compile)), faintRoot, include, out, err, objRoot, wx_root, debug=opts.debug, num_parallell=parallell_compiles )
    else:
        print("* Compiling")
        print(" Up to date.")

    if do_link:
        print("* Linking")
        files = enumerate_obj( objRoot )
        files.extend([
            os.path.join(objRoot2, "angle.obj"),
            os.path.join(objRoot2, "bitmap.obj"),
            os.path.join(objRoot2, "color.obj"),
            os.path.join(objRoot2, "draw-source.obj"),
            os.path.join(objRoot2, "faint-cairo-stride.obj"),
            os.path.join(objRoot2, "geo-func.obj"),
            os.path.join(objRoot2, "gradient.obj"),
            os.path.join(objRoot2, "index.obj"),
            os.path.join(objRoot2, "intpoint.obj"),
            os.path.join(objRoot2, "intrect.obj"),
            os.path.join(objRoot2, "intsize.obj"),
            os.path.join(objRoot2, "math-constants.obj"),
            os.path.join(objRoot2, "mouse.obj"),
            os.path.join(objRoot2, "pathpt.obj"),
            os.path.join(objRoot2, "pattern.obj"),
            os.path.join(objRoot2, "point.obj"),
            os.path.join(objRoot2, "primitive.obj"),
            os.path.join(objRoot2, "radii.obj"),
            os.path.join(objRoot2, "rect.obj"),
            os.path.join(objRoot2, "size.obj"),
            os.path.join(objRoot2, "zoom-level.obj"),
                  ])

        print(" Object files: " + str(len(files)))
        files = [ os.path.join( objRoot, file ) for file in files ]
        link_test(files, faintRoot, wx_root, cairo_lib, python_lib, pango_lib, glib_lib, debug=opts.debug)

    if platform == 'msw' and opts.version != unknown_version_str:
        print("* Generating %s installer" % opts.version)
        oldDir = os.getcwd()
        os.chdir("../installer")
        nsiFile = gennsis.run(opts.version)
        create_installer(makensis, nsiFile)
        os.chdir(oldDir)

if __name__ == '__main__':
    build()
