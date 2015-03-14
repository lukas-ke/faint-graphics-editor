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

import os
import subprocess
import time
import sys
from build_sys.util.util import changed, print_timing

obj_ext = '.obj'

cl_common_switches = [
    "EHsc", # Exception mode sc
    "GF", # String pooling
    "GT", # Fiber-safe thread local storage
    "Gm-", # Disable minimal rebuild
    "Gy", # Disable function level linking (why?)
    "Oi", # Intrinsic functions (faster but larger)
    "W4", # Warning level
    "WX", # Treat warnings as errors
    "Y-", # Ignore precompiled header options
    "fp:precise", # Floating point model
    "nologo", # No startup banner
    "Gd", # cdecl calling convention
    "analyze-",
    "errorReport:queue",
    "Zc:forScope", # ISO-Conformance
    "Zc:wchar_t", # ISO-Conformance

    # /Za disables Microsoft extensions. Not used here because it causes many
    # warnings in wxWidgets headers and Microsoft's last developer says its
    # broken:
    # http://clang-developers.42468.n3.nabble.com/MSVC-Za-considered-harmful-td4024306.html
]

cl_release_switches = [
    "MD",
    #"GL", # Whole program optimization, requires LTCG for linker
    "GS-", # Disable Buffer overrun check
    "Ot", # Favor fast code
    "Ox", # Optimize, Favor speed
    #"P", # Keep preprocessed file
    #"Oy-", # Do not suppress frame pointer (old 386 optimization?)
]

cl_debug_switches = [
    "MDd", # Link windows Multi-threaded static windows runtime - debug
    "RTC1", # Runtime-checks
    "GS", # Buffer security check
    "Od", # Disable optimization
    "Zi", # Full debugging information (pdb?)
]

def get_cl_switches(debug):
    switches = cl_common_switches[:]
    if debug:
        switches.extend(cl_debug_switches)
    else:
        switches.extend(cl_release_switches)
    return switches

def compile(in_list,opts, out, err, debug):
    assert(len(set(in_list)) == len(in_list))
    fileList = [f.replace('/', '\\') for f in in_list]

    assert(len(set(fileList)) == len(fileList))
    includes = [ "/I" + include.replace("/", "\\") for include in
                 opts.include_folders ]

    # VC has no system include option (afaik)
    includes.extend([ "/I" + include.replace("/", "\\") for include in
                 opts.system_include_folders])

    defines = ["_LIB" ,
               "__WXMSW__" ,
               "WXBUILDING" ,
               "wxUSE_GUI=1" ,
               "wxUSE_BASE=1" ,
               "_UNICODE" ,
               "_WINDOWS" ,
               "NOPCH" ,
               "_CRT_SECURE_NO_WARNINGS" ,
               "UNICODE",
               "WIN32",

               # Disable Microsofts iterator debugging to allow linking with
               # release Python.
               # This might require disabling also for wxWidgets, in
               # wxWidgets/build/msw/config.vc, set
               # CXXFLAGS = /D_ITERATOR_DEBUG_LEVEL=0
               "_ITERATOR_DEBUG_LEVEL=0"]

    defines = ['/D%s ' % define for define in defines]
    defines.append('/DFAINT_MSW')
    switches = get_cl_switches(debug)
    default = " ".join(["/%s" % switch for switch in switches ])
    args = " ".join( defines ) + " " + default + " " +  " ".join( includes )

    if opts.forced_include is not None:
        args += " /FI " + opts.forced_include

    cmd = "cl.exe " + args
    if opts.parallell_compiles > 0:
        cmd = cmd + " /MP%d" % opts.parallell_compiles
    cmd += " /c " + " ".join(fileList)
    sys.stdout.flush()
    old = os.getcwd()
    os.chdir(opts.get_obj_root() )
    try:
        cl = subprocess.Popen(cmd, 0, None, None, out, err)
    except FileNotFoundError as e:
        print("Compilation failed: cl.exe not in PATH?")
        exit(1)

    os.chdir( old )
    if cl.wait() != 0:
        print("Compilation failed")
        exit(1)

def compile_resources(faintRoot, target_folder, wxRoot, out, err):
    # Fixme: Pass .rc path instead
    targetFile = os.path.join(target_folder, "faint.res")
    sourceFile = os.path.join(faintRoot.replace("/", "\\"), "graphics", "faint.rc")

    if not changed(sourceFile, targetFile):
        return targetFile

    cmd = 'rc /fo "%s" /I"%s\\include" "%s\\graphics\\faint.rc"' % (
        targetFile, wxRoot, faintRoot)
    resourceCompile = subprocess.Popen(cmd, 0, None, None, out, err)

    if resourceCompile.wait() != 0:
        print("Compiling resources failed")
        exit(1)

    return targetFile

wxlibs_debug = [
    "wxbase31ud.lib",
    "wxbase31ud_net.lib",
    "wxbase31ud_xml.lib",
    "wxexpatd.lib",
    "wxjpegd.lib",
    "wxmsw31ud_adv.lib",
    "wxmsw31ud_aui.lib",
    "wxmsw31ud_core.lib",
    "wxmsw31ud_gl.lib",
    "wxmsw31ud_html.lib",
    "wxmsw31ud_media.lib",
    "wxmsw31ud_propgrid.lib",
    "wxmsw31ud_qa.lib",
    "wxmsw31ud_richtext.lib",
    "wxmsw31ud_stc.lib",
    "wxmsw31ud_xrc.lib",
    "wxpngd.lib",
    "wxregexud.lib",
    "wxscintillad.lib",
    "wxtiffd.lib",
    "wxzlibd.lib" ]

wxlibs_release = [
    "wxbase31u.lib",
    "wxbase31u_net.lib",
    "wxbase31u_xml.lib",
    "wxexpat.lib",
    "wxjpeg.lib",
    "wxmsw31u_adv.lib",
    "wxmsw31u_aui.lib",
    "wxmsw31u_core.lib",
    "wxmsw31u_gl.lib",
    "wxmsw31u_html.lib",
    "wxmsw31u_media.lib",
    "wxmsw31u_propgrid.lib",
    "wxmsw31u_qa.lib",
    "wxmsw31u_richtext.lib",
    "wxmsw31u_stc.lib",
    "wxmsw31u_xrc.lib",
    "wxpng.lib",
    "wxregexu.lib",
    "wxscintilla.lib",
    "wxtiff.lib",
    "wxzlib.lib" ]

def get_wxlibs(debug):
    return wxlibs_debug if debug else wxlibs_release

def create_lib_paths_string(libs):
    return "/LIBPATH:" + " /LIBPATH:".join(libs)

def to_subsystem_flag(subsystem):
    return ("/SUBSYSTEM:WINDOWS" if subsystem == "windows"
            else "/SUBSYSTEM:CONSOLE")

def clean_vc_nonsense(bo):
    root_name = os.path.join(bo.project_root, bo.get_out_name())
    for ext in (".exp", ".lib", ".exe.manifest"):
        fn = root_name + ext
        if os.path.exists(fn):
            os.remove(fn)

def link(files, opts, out, err, debug):
    resFile = compile_resources(opts.project_root, opts.get_obj_root(),
                                opts.extra_resource_root,
                                out, err)
    old = os.getcwd()
    os.chdir(opts.project_root)
    flags = "/NOLOGO"
    wxlibs = get_wxlibs(debug)
    out_name = opts.get_out_name()
    if debug:
        flags += " /OUT:%s.exe /DEBUG /PDB:%s.pdb" % (out_name,
                                                      out_name)
    else:
        flags += " /OUT:%s.exe" % (out_name)

    lib_paths_string = create_lib_paths_string(opts.lib_paths)

    cmd = "Link.exe " + flags + " " + to_subsystem_flag(opts.msw_subsystem) + " " + "/OPT:REF " + lib_paths_string + " " + " ".join(files) + " " + " ".join(wxlibs) + " comctl32.lib rpcrt4.lib shell32.lib gdi32.lib kernel32.lib gdiplus.lib cairo.lib comdlg32.lib user32.lib Advapi32.lib Ole32.lib Oleaut32.lib Winspool.lib pango-1.0.lib pangocairo-1.0.lib pangoft2-1.0.lib pangowin32-1.0.lib gio-2.0.lib glib-2.0.lib gmodule-2.0.lib gobject-2.0.lib gthread-2.0.lib " + resFile
    linker = subprocess.Popen(cmd, stdout=out, stderr=err)
    if linker.wait() != 0:
        print("Linking failed.")
        exit(1)

    if opts.msw_subsystem == "windows":
        # The manifest compiler can sometimes not write to the exe after
        # the linking, maybe waiting a tiny bit helps
        time.sleep(1)

        # Embed the manifest to get the correct common controls version etc.
        # See: http://msdn.microsoft.com/en-us/library/ms235591(v=vs.80).aspx
        manifestCmd = ('mt.exe -manifest %s.exe.manifest '
                       '-outputresource:%s.exe;1') % (out_name, out_name)
        for attempt in range(3):
            embedManifest = subprocess.Popen(manifestCmd, stdout=out, stderr=err)
            if embedManifest.wait() != 0:
                print("Embedding Manifest failed.")
                time.sleep(1)
            else:
                clean_vc_nonsense(opts)
                break

        else:
            print("Totally failed embedding Manifest.")
    else:
        clean_vc_nonsense(opts)

    os.chdir( old )

def create_installer(makensis, nsifile):
    nsis = subprocess.Popen(makensis + " " + nsifile)
    if nsis.wait() != 0:
        print("Installer creation failed.")
        exit(1)
