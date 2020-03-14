# Building Faint on Linux

These instructions were tested with xubuntu 18.04.4 (AMD64) in Oracle
VirtualBox.

For simplicity, these instructions assume that Faint and dependencies
are cloned into ~/dev.

## Prerequisites
You may need to get these packages:
* gtk+-2.0
* libgtk-3-dev

**Note** Either g++ or clang are required.
These instructions were tested with g++ 9.2.1.

## 1. wxWidgets
wxWidgets is the GUI toolkit used for Faint.

### Clone the wxWidgets trunk

    cd ~/dev
    git clone https://github.com/wxWidgets/wxWidgets.git wxWidgets
    git submodule update --init --recursive

### Build wxWidgets

    cd ~/dev/wxWidgets
    ./configure --disable-shared --enable-monolithic --enable-aui
    make

(If tiff gives you problems, pass `--without-libtiff` to configure).

## 2. Python
Python is used for building Faint and as the embedded scripting language of Faint.

### Install the Python dev-files
Python3.8 and its development headers are required.

    sudo apt install python3.8
    sudo apt install python3.8-dev

## 3. Faint
And finally...

### Clone Faint
    cd ~/dev
    git clone https://github.com/lukas-ke/faint-graphics-editor.git

### Create build.cfg
The file `build.cfg` contains paths to Faint-depdendencies. Generate
the config file as follows:

    cd ~/dev/faint-graphics-editor/build
    ./build.py

This creates the build.cfg file. Edit it so that it points out
dependencies correctly, example:

    [folders]
    wx_root=~/dev/wxWidgets
    cairo_include=/usr/include/cairo
    cairo_lib=
    python_include=/usr/include/python3.8
    python_lib=/usr/lib/python3.8/config-3.8-x86_64-linux-gnu
    pango_include=/usr/include/pango-1.0
    pango_lib=
    glib_include=/usr/include/glib-2.0
    glib_lib=
    glib_config_include=/usr/lib/x86_64-linux-gnu/glib-2.0/include
    pnglib_include=/usr/include/libpng16
    [other]
    compiler=gcc
    parallell_compiles=4
    etags_folder=

`compiler` can be either gcc or clang.

### Build Faint
Run build.py again to build Faint.

    cd ~/dev/faint-graphics-editor/build
    ./build.py

### Start Faint

    cd ~/dev/faint-graphics-editor/
    ./faint
