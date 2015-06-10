# Building Faint on Windows using Visual Studio 2015

I use the Visual Studio 2015 RC community edition for building Faint
in Windows.

Note that since I only Visual Studio for compiling, not for editing,
there is no Visual Studio project or solution included with the Faint
sources. Instead I use a Python script which invokes the Visual Studio
compiler.

Many of the instructions below are done in the command prompt. The
build steps require that the Visual Studio command prompt is used -
and not the x64 variant!

The instructions are written assuming that "c:\dev" is used as a root
for checkouts etc., adapt this to your preferences

## Prerequisites
* [Visual C++ Community Edition](https://www.visualstudio.com/en-us/downloads/visual-studio-2015-downloads-vs.aspx)
* [git](https://git-scm.com/)

## 1. wxWidgets
wxWidgets is the GUI toolkit used for Faint.

### Clone the wxWidgets trunk
    cd c:\dev
    git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets

This clones the wxWidgets source to c:\dev\wxWidgets.

### Build wxWidgets
    cd c:\dev\wxWidgets\build\msw
    nmake BUILD=release SHARED=0 -f makefile.vc

## 2. Python
Python is used both for generating some code when building Faint and
as the embedded scripting language of Faint. Faint requires Python 3.4.

### Install Python 3.4
http://www.python.org/download/

## 3. Pango and Cairo
Faint uses Cairo for rendering vector graphics, and Pango for
rendering text.

### Get the Pango and Cairo development files
http://www.gtk.org/download/win32.php

The required items are listed below. Unzip these somewhere convenient.

**Required Dev** These are required for building Faint, and contain
libraries and headers. These will be indicated by the build.cfg (see
step 4).

* GLib Dev
* Pango Dev
* Cairo Dev

**Required Run time** DLL:s from these zips are needed to run Faint.

* Freetype runtime
* gettext runtime
* Cairo runtime
* expat runtime
* Fontconfig runtime
* GLib runtime
* Pango runtime
* libpng runtime
* zlib runtime

## 4. Faint
Without further ado...

### Clone the Faint trunk (read only)
    cd c:\dev
    git clone git@github.com:lukas-ke/faint-graphics-editor.git faint-graphics-editor

This clones the Faint source to c:\dev\faint-graphics-editor.

### Create build.cfg
The build.cfg contains paths to Faint dependencies. Generate the
build-config as follows:

    cd c:\dev\faint-graphics-editor\build
    build.py

This creates the build.cfg file. Edit it so that it correctly points out
wxWidgets, Pango and Cairo.

Example build.cfg:

    [folders]
    wx_root=c:\dev\wxWidgets
    cairo_include=c:\dev\cairo-dev_1.10.2-2_win32\include\cairo
    cairo_lib=c:\dev\cairo-dev_1.10.2-2_win32\lib
    python_include=c:\Python34\include
    python_lib=c:\Python34\libs
    pango_include=c:\dev\pango-dev_1.29.4-1_win32\include\pango-1.0
    pango_lib=c:\dev\pango-dev_1.29.4-1_win32\lib
    pnglib_include=c:\dev\wxWidgets\src\png
    glib_include=c:\dev\glib-dev_2.28.8-1_win32\include\glib-2.0
    glib_lib=c:\dev\faintdep\glib-dev_2.28.8-1_win32\lib
    glib_config_include=c:\dev\glib-dev_2.28.8-1_win32\lib\glib-2.0\include
    [nsis]
    makensis=
    [other]
    parallell_compiles=4
    etags_folder=

The entries makensis and etags\_folder can be left blank, they are
only used for generating an installer and tags for the emacs editor
respectively.

### Build Faint
Run build.py again

    build.py

Faint should now build, producing the executable "Faint.exe" in the
root-folder (e.g c:\\dev\\faint\\faint.exe).

### Copy runtime libraries
Copy the following dll:s from the Runtime packages downloaded in (3)
to the Faint root folder (e.g. c:\dev\faint-graphics-editor):

* freetype6.dll
* intl.dll
* libcairo-2.dll
* libexpat-1.dll
* libfontconfig-1.dll
* libgio-2.0-0.dll
* libglib-2.0-0.dll
* libgmodule-2.0-0.dll
* libgobject-2.0-0.dll
* libgthread-2.0-0.dll
* libpango-1.0-0.dll
* libpangocairo-1.0-0.dll
* libpangoft2-1.0-0.dll
* libpangowin32-1.0-0.dll
* zlib1.dll

### Start
Run the Faint executable (Faint.exe). Faint should start.
