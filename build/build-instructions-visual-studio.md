# Building Faint on Windows using Build Tools for VS2019

I use the "Build Tools for Visual Studio 2019" for building Faint in
Windows.

Note that, since I only use the build tools, there is no Visual Studio
project or solution included with the Faint sources. Instead I use a
Python script which invokes the Visual Studio compiler directly.

Many of the instructions below are done in the command prompt. The
build steps require that the **Developer Command Prompt for VS 2019**
is used.

The instructions are written assuming that "c:\dev" is used as a root
for checkouts etc., adapt this to your preferences.

## Prerequisites

### Build tools for Visual Studio 2019

https://visualstudio.microsoft.com/downloads/

Download the build tools installer and run it. The installer is named
something like `vs_buildtools_<numbers>.exe`.

At a minimum these components must be selected in the "Build
tools"-installer:

* C++-build tools core features
* C++-2019-redistributable update
* Windows 10 SDK
* MSVC v142

Note: Visual Studio 2019 Community (or any other edition of 2019)
probably works as well, as long as the above features are included,
but I have not tried it.

### Vcpkg
https://github.com/microsoft/vcpkg

Microsoft package manager, used for getting and building some Faint dependencies.

Clone it and run bootstrap:

    cd c:\dev
    git clone https://github.com/Microsoft/vcpkg
    bootstrap-vcpkg.bat

## 1. wxWidgets
wxWidgets is the GUI toolkit used for Faint.

### Clone the wxWidgets trunk

    cd c:\dev
    git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets

This clones the wxWidgets source to c:\dev\wxWidgets.

### Build wxWidgets

    cd c:\dev\wxWidgets\build\msw
    nmake BUILD=release SHARED=0 -f makefile.vc
	
Note: You can probably use Vcpkg instead, but I have not tried this.
See https://www.wxwidgets.org/blog/2019/01/wxwidgets-and-vcpkg/

## 2. Python 3
Python is used both for generating some code when building Faint and
as the embedded scripting language of Faint. Faint requires Python3,
and was most recently tested with 3.8.2.

### Install Python 3
http://www.python.org/download/

Get the x86 (32-bit)-version, since that's how Faint is built
currently.

## 3. Pango and Cairo
Faint uses Cairo for rendering vector graphics, and Pango for
rendering text.

Use Vcpkg to download and build Pango, Cairo and related dependencies.
Simply getting cairo seems to bring with it a lot of the dependencies
(e.g. Pango):

    cd c:\dev\vcpkg
    vcpkg install cairo:x86-windows

If any of the dependencies cairo, pango, glib or libpng16 are missing
get them explicitly for x86-windows with vcpkg.

## 4. Faint
And finally...

### Clone Faint

    cd c:\dev
    git clone https://github.com/lukas-ke/faint-graphics-editor.git faint-graphics-editor

This clones the Faint source to c:\dev\faint-graphics-editor.

### Create build.cfg

The build.cfg contains paths to Faint dependencies. Generate the
build-config as follows:

    cd c:\dev\faint-graphics-editor\build
    build.py

This creates the build.cfg file. Edit it so that it correctly points
out wxWidgets, Pango and Cairo. If you used Vcpkg, it should look like
the example below (except for any initial path differences and Python
install path).

Example build.cfg:

    [folders]
    wx_root=c:\dev\wxWidgets
    cairo_include=C:\dev\vcpkg\installed\x86-windows\include\cairo
    cairo_lib=C:\dev\vcpkg\installed\x86-windows\lib
    python_include=C:\Users\UserName\AppData\Local\Programs\Python\Python38-32\include
    python_lib=C:\Users\UserName\AppData\Local\Programs\Python\Python38-32\libs
    pango_include=C:\dev\vcpkg\installed\x86-windows\include
    pango_lib=C:\dev\vcpkg\installed\x86-windows\lib
    glib_include=C:\dev\vcpkg\installed\x86-windows\include
    glib_lib=C:\dev\vcpkg\installed\x86-windows\lib
    glib_config_include=C:\dev\vcpkg\installed\x86-windows\include
    pnglib_include=c:\dev\wxWidgets\src\png
    [nsis]
    makensis=
    [other]
    parallell_compiles=4
    etags_folder=

The entries makensis and etags\_folder can be left blank, they are
only used for generating an installer and tags for the emacs editor
respectively.

### Build Faint
Run build.py again (from the Developer command prompt):

    cd c:\dev\faint-graphics-editor\build
    build.py

Faint should now build, producing the executable "Faint.exe" in the
root-folder (e.g c:\\dev\\faint-graphics-editor\\faint.exe).

### Copy runtime libraries
Copy the following dll to the Faint root folder
(e.g. `c:\dev\faint-graphics-editor`):

* python38.dll 

from `C:\Users\UserName\AppData\Local\Programs\Python\Python38-32`(or
wherever you've installed Python).

Also copy these dll:s:

* bz2.dll
* cairo.dll
* expat.dll
* fontconfig.dll
* freetype.dll
* glib-2.dll
* gobject-2.dll
* harfbuzz.dll
* libcharset.dll
* libffi.dll
* libiconv.dll
* libintl.dll
* libpng16.dll
* pango-1.dll
* pangocairo-1.dll
* pangoft2-1.dll
* pangowin32-1.dll
* pcre.dll
* zlib1.dll

from `vcpkg\installed\x86-windows\bin` (they were created by build
step 3).

### Start
Run the Faint executable (Faint.exe). Faint should start.
