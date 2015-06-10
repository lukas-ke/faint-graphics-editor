# Building Faint on Linux

For simplicity, these instructions assume that Faint and dependencies
are cloned into ~/dev.

These instructions were tested with xubuntu 14.10 (64-bit) in Oracle
VirtualBox.

## Prerequisites
You may need to get these packages:
* build-essentials
* gtk+-2.0

**Note** Either gcc-4.9 or clang-3.6 (or later) are required.

## 1. wxWidgets
wxWidgets is th GUI toolkit used for Faint.

### Clone the wxWidgets trunk

    cd ~/dev
    git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets

### Build wxWidgets

    cd ~/dev/wxWidgets
    ./configure --disable-shared --enable-monolithic --enable-aui
    make

## 2. Python
Python is used for generating some code when building Faint, as a
build script and as the embedded scripting language of Faint.

### Install the Python dev-files
Python3.4 and its development headers are required.

    sudo apt-get install python3.4
    sudo apt-get install python3.4-dev

## 3. Faint
Finally!

### Clone the Faint trunk
    cd c:\dev
    git clone git@github.com:lukas-ke/faint-graphics-editor.git faint-graphics-editor

### Create build.cfg

The file `build.cfg` contains paths to Faint-depdendencies. Generate
the config file as follows:

    cd ~/dev/faint-graphics-editor/build
    python3 build.py

This creates the build.cfg file. Edit it so that it points out
dependencies correctly, example:

    [folders]
    wx_root=~/dev/wxWidgets
    cairo_include=/usr/include/cairo
    cairo_lib=
    python_include=/usr/include/python3.4
    python_lib=/usr/lib/python3.4/config-3.4m-x86_64-linux-gnu
    pango_include=/usr/include/pango-1.0
    pango_lib=
    pnglib_include=/usr/include/libpng12
    glib_include=/usr/include/glib-2.0
    glib_lib=
    glib_config_include=/usr/lib/x86_64-linux-gnu/glib-2.0/include
    [other]
    compiler=gcc
    parallell_compiles=4
    etags_folder=

### Build Faint

Run build.py again to build Faint.

    cd ~/dev/faint-graphics-editor/build
    python3 build.py

### Start Faint

    cd ~/dev/faint-graphics-editor/build
    ./faint
