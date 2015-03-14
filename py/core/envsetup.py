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

import sys
from code import InteractiveConsole
import copy
import ifaint
import os
import __main__

class NamedFunc:
    def __init__(self, name, function):
        self.function = function
        self.__name__ = name

    def __call__(self, *args):
        self.function(*args)
    def __str__(self):
        return self.name

# Will be True while a command is run silently (e.g. from a keybind,
# not the interpreter)
_g_silent = False

# Will be True when a command has printed output, until a new prompt
# is displayed
_g_printed = False

class _mywriter:
    """[Faint internal] Replacement for stderr and stdout which
    directs output to the Faint interpreter window."""

    def write(self, str):
        if _g_silent:
            # Output is being made through a keypress or similar.  The
            # interpreter is probably in enter command mode (>>>).
            # So: Create a new line if this is the first print of a
            # batch (or the only print), so the output appears on a
            # prompt-free line
            global _g_printed
            if not _g_printed:
                ifaint.int_faint_print("\n")
                # Signal that a print occured, to show that
                # a new prompt is needed etc.
                _g_printed = True

        ifaint.int_faint_print(str)
sys.stdout = _mywriter()
sys.stderr = _mywriter()

# Python-interpreter, used for submitting Python strings for compilation.
# Also accepts partial commands which can be added on until completed.
_console = InteractiveConsole(__main__.__dict__)

def interpreter_new_prompt():
    """[Faint internal] Creates a new prompt (>>>) in the interpreter,
    and resets command states _g_printed and _g_silent"""
    global _g_printed
    global _g_silent
    _g_printed = False
    _g_silent = False
    ifaint.int_ran_command()

def push(str_):
    """[Faint internal] Push a line of text to the Python interpreter process"""

    if _console.push(str_):
        # More strings must be pushed before command can execute
        ifaint.int_incomplete_command()
    else:
        # Command executed. Inform interested parties.
        if str_ != "":
            push("")
        else:
            interpreter_new_prompt()

ifaint.push = push

# Fixme: Must silent commands be complete? What happens if not?
def push_silent(str):
    """[Faint internal] Push comands without outputting any feedback
    (e.g. for commands bound to keys)"""
    global _g_silent
    global _g_printed

    try:
        # Make the fact that the command is run silently
        # globally visible
        _g_silent = True
        for line in str.split('\n'):
            _console.push(line)
    finally:
        _g_silent = False

        if _g_printed:
            # Output was made, so the interpreter must
            # add a new prompt-line
            interpreter_new_prompt()

def keypress(key, modifier=0):
    """[Faint internal] Called when a key is pressed in Faint"""
    fn = ifaint._binds.get((key, modifier), None)
    if fn is not None:
        fn()

# Required for access outside envsetup.py
ifaint.keypress = keypress

class binder:
    """[Faint internal] Used for interactive function binding:

    This is required because function binding is done in steps:
    A global instance of this class is used to store the function that
    is to be bound until the user presses the key the function will be
    bound to to."""
    def __init__(self):
        self._func = None
        self.bindGlobal = False

    def bind(self, function, bindGlobal=False):
        """Interactively connect a Python-function to a keyboard key.

        Example use:
        >>> bind(zoom_in)
        [ Press key ]"""
        self.bindGlobal = bindGlobal

        if callable(function):
            self._func = function
            ifaint.int_get_key()
        else:
            raise TypeError("%s is not callable." % str(function))

    def bind_global(self, function):
        """Like bind, but ignores which Faint window has focus."""
        self.bind(function, True)

    def bind2(self, key, modifiers=0):
        """[Faint internal] Bind is a two step operation, since it requires
        intervention from the console (a keypress). This is step two, which
        delegates to bindk"""
        if self._func is None:
            raise TypeError("No function ready for binding.")
        bindk(key, self._func, modifiers, self.bindGlobal)
        self._func = None

    def c_dummy(self): pass

_b = binder()

ifaint.bind = _b.bind
ifaint.bind2 = _b.bind2
ifaint.bind_global = _b.bind_global

def bindk(keycode, function, modifiers=0, bindGlobal=False):
    """Connect a function to the key specified by keycode.
    For interactive binding, try bind(function) instead."""
    if not callable(function):
        raise ValueError("Bound argument must be a function")

    ifaint._binds[ (keycode,modifiers) ] = function
    if bindGlobal:
        ifaint.int_bind_key_global(keycode, modifiers)
    else:
        ifaint.int_bind_key(keycode, modifiers)
ifaint.bindk = bindk

def bindk_global(keycode, function, modifiers=0):
    """Binds a function to a key globally, i.e. the bind will trigger
    regardless of which window or control within Faint has focus."""
    bindk(keycode, function, modifiers, True)
ifaint.bindk_global = bindk_global

def bindc(char, function, modifier=0):
    """Connect a function to the key specified by char.
    For interactive binding, try bind(function) instead."""
    bindk(ord(char.upper()), function, modifier)
ifaint.bindc = bindc

def unbindc(char):
    """Unbind a key specified by a character"""
    unbindk(ord(char))
ifaint.unbindc = unbindc

def unbindk(key, verbose=True, modifiers=0):
    """Remove the bind from the given keycode"""
    if (key, modifiers) in ifaint._binds:
        if verbose:
            print("Unbinding: " + str(key) + " " + ifaint._binds[ (key, modifiers) ].__name__)
        del ifaint._binds[ (key, modifiers) ]
        ifaint.int_unbind_key(key, modifiers)
ifaint.unbindk = unbindk

def unbindf(function):
    """Removes all keybinds to the specified function."""
    d = ifaint._binds
    keys = list(d.keys())
    for key in keys:
        if d[key] == function:
            del d[key]
ifaint.unbindf = unbindf

def printDoc(docstr, indent):
    lines = docstr.split('\n')
    if len(lines) == 1:
        print('  "' + lines[0].strip() + '"')
        return

    for line in lines[:-1]:
        print('  ' + line.strip())
    print('  ' + lines[-1] + '"')

def special_char(keycode):
    return not (40 < keycode < 150)


mod = ifaint.mod
key = ifaint.key

def key_text(keycode, modifiers, numeric):
    if numeric:
        keyLabel = str(keycode)
    else:
        keyLabel = ifaint.int_get_key_name(keycode)

    if modifiers == 0:
        return keyLabel
    if modifiers & mod.alt == mod.alt:
        keyLabel = "Alt+" + keyLabel
    if modifiers & mod.shift == mod.shift:
        keyLabel = "Shift+" + keyLabel
    if modifiers & mod.ctrl == mod.ctrl:
        keyLabel = "Ctrl+" + keyLabel
    return keyLabel

def binds(verbose=False, numeric=False):
    """List what keyboard shortcuts are connected to Python functions.
    With verbose=True, doc strings are included.
    With numeric=True, key code values are shown instead of characters or
    key names. See also "bind" for help on binding."""

    # Dict to keep track of printed doc-strings under verbose,
    # to avoid repeating them for multiple binds
    printed = {}
    for key, modifiers in sorted(ifaint._binds, key=lambda bind : bind[0]):
        func = ifaint._binds[(key, modifiers)]
        print (" " + key_text(key, modifiers, numeric) + ": " + func.__name__)

        if verbose and func.__doc__ is not None:
            if (func in printed):
                print(" (See " + str(printed[func]) + " above)")
            else:
                printed[func] = key_text(key, modifiers, numeric)
                printDoc(func.__doc__, 2)
    if not verbose and not numeric:
        print("Use binds(t) to print doc-strings for functions.")
ifaint.binds = binds

# For convenience
ifaint.f = False
ifaint.t = True

ifaint.window = ifaint.FaintWindow()
ifaint.interpreter = ifaint.FaintInterpreter()
ifaint.palette = ifaint.FaintPalette()
ifaint.app = ifaint.FaintApp()

def load_test(filename):
    print("Load: " + filename)

def save_test(filename):
    print("Save: " + filename)

def pick_color_fg():
    try:
        ifaint.set_fg(ifaint.get_paint(*ifaint.get_mouse_pos()))
    finally:
        return

def pick_color_bg():
    try:
        ifaint.set_bg(get_paint(*get_mouse_pos()))
    finally:
        return

def _pick_pattern_fg():
    pattern = ifaint.Pattern(ifaint.get_mouse_pos(), ifaint.get_active_image().get_bitmap())
    pattern.set_object_aligned(True)
    ifaint.set_fg(pattern)
ifaint.pick_pattern_fg = _pick_pattern_fg;

def _select_top_object():
    objects = ifaint.get_objects()
    if len(objects) > 0:
        ifaint.tool(0)
        ifaint.set_layer(1)
        i = ifaint.get_active_image()
        i.select(objects[-1])

ifaint.select_top_object = _select_top_object

def _select_bottom_object():
    objects = ifaint.get_objects()
    if len(objects) > 0:
        i = ifaint.get_active_image()
        i.select(objects[0])

def center_on_selected():
    """Centers the view on the selected object."""
    objects = ifaint.get_selected()
    if len(objects) > 0:
        ifaint.center(objects[0].tri().center())

def _toggle_selection_type():
    """Toggles between raster and object selection"""
    if ifaint.tool() == 0:
        ifaint.set_layer(1 - ifaint.get_layer())
    else:
        ifaint.tool_selection()

ifaint.toggle_selection_type = _toggle_selection_type

ifaint.raster_layer = NamedFunc("raster_layer", lambda: ifaint.set_layer(0))
ifaint.object_layer = NamedFunc("object_layer", lambda: ifaint.set_layer(1))

# Function used for producing the arguments for subprocess.Popen for
# browsing to a file.
#
# The return value is used for subprocess.Popen, so this should return
# a list or a string (list is preferable, except on Windows, see
# http://docs.python.org/2/library/subprocess.html#converting-argument-sequence
ifaint.external_browse_to_file_command = None

def _file_browse_cmd_msie(file_path):
    if os.path.exists(file_path):
        return 'explorer /select,"%s"' % file_path

    # The file is missing (probably renamed or moved), explore
    # the directory instead, if it exists
    dir_path = os.path.dirname(file_path)
    if os.path.exists(dir_path):
        return u'explorer "%s"' % dir_path
    else:
        return ""

ifaint.file_browse_cmd_msie = _file_browse_cmd_msie
ifaint.file_browse_cmd_thunar = lambda f: ('thunar', os.path.dirname(f))
ifaint.file_browse_command_nautilus = lambda f: ('nautilus', os.path.dirname(f))

if os.name == 'nt':
    ifaint.external_browse_to_file_command = ifaint.file_browse_cmd_msie

def _browse_to_active_file():
    """Calls ifaint.external_browse_to_file_command with the filename of the
    active image."""

    if ifaint.external_browse_to_file_command is None:
        raise ValueError("ifaint.external_browse_to_file_command not configured.")
    filename = ifaint.get_active_image().get_filename()
    if filename is not None:
        import subprocess
        cmd_string = ifaint.external_browse_to_file_command(filename)
        if len(cmd_string) != 0:
            cmd = ifaint.external_browse_to_file_command(filename)
            subprocess.Popen(cmd, shell=True)

ifaint.browse_to_active_file = _browse_to_active_file

#
# Forwarding functions for object creation to the active canvas
#
def _Spline(*args, **kwArgs):
    return ifaint.get_active_image().Spline(*args, **kwArgs)
ifaint.Spline = _Spline

def _Rect(*args, **kwArgs):
    return ifaint.get_active_image().Rect(*args, **kwArgs)
ifaint.Rect = _Rect

def _Polygon(*args, **kwArgs):
    return ifaint.get_active_image().Polygon(*args, **kwArgs)
ifaint.Polygon = _Polygon

def _Line(*args, **kwArgs):
    return ifaint.get_active_image().Line(*args, **kwArgs)
ifaint.Line = _Line

def _Ellipse(*args, **kwArgs):
    return ifaint.get_active_image().Ellipse(*args, **kwArgs)
ifaint.Ellipse = _Ellipse

def _Group(*args, **kwArgs):
    return ifaint.get_active_image().Group(*args, **kwArgs)
ifaint.Group = _Group

def _Raster(*args, **kwArgs):
    return ifaint.get_active_image().Raster(*args, **kwArgs)
ifaint.Raster = _Raster

def _Text(*args, **kwArgs):
    return ifaint.get_active_image().Text(*args, **kwArgs)
_Text.__doc__ = "Adds a text object to the active image. Equivalent to get_active_images().Text(*args, **kwArgs)\n\nCanvas.Text:\n" + "".join(["  %s\n" % line for line in ifaint.Canvas.Text.__doc__.split("\n")])
ifaint.Text = _Text

def _Path(*args, **kwArgs):
    return ifaint.get_active_image().Path(*args, **kwArgs)
ifaint.Path = _Path

# Custom
def _center_on_cursor():
    image = ifaint.get_active_image()
    pos = image.get_mouse_pos()
    image.center(*pos)
ifaint.center_on_cursor = _center_on_cursor

def _scroll_top_left(*args, **kwArgs):
    ifaint.scroll_max_left()
    ifaint.scroll_max_up()
ifaint.scroll_top_left = _scroll_top_left

def _scroll_bottom_right(*args, **kwArgs):
    ifaint.scroll_max_right()
    ifaint.scroll_max_down()
ifaint.scroll_bottom_right = _scroll_bottom_right

# Other Forwarding functions
def _apply_images(func, images):
    if len(images) == 0:
        return func(ifaint.get_active_image())
    if len(images) == 1:
        first_obj = images[0]
        if first_obj.__class__ == ifaint.Canvas:
            func(first_obj)
            return
        else:
            for img in first_obj:
                if img.__class__ != ifaint.Canvas:
                    raise TypeError("Non-canvas specified")
            for img in first_obj:
                func(img)
    else:
        for img in images:
            if img.__class__ != ifaint.Canvas:
                raise TypeError("Non-canvas specified")
        for img in images:
            func(img)

def _forwarder(func):
    l = lambda *images : _apply_images(func, images)
    l.__name__ = func.__name__
    l.__doc__ = "Forward to Canvas." + func.__doc__
    return l

def _active(func):
    l = lambda *args, **kwArgs : func(ifaint.get_active_image(), *args, **kwArgs)
    l.__name__ = func.__name__
    l.__doc__ = "Single-forward to Canvas." + func.__doc__
    return l

_c = ifaint.Canvas

# Forwarding functions for no-argument functions provide
# func() - apply to active image
# func(i1, i2) and func(imagelist) - apply to specified images
ifaint.auto_crop = _forwarder(_c.auto_crop)
ifaint.context_crop = _forwarder(_c.context_crop)
ifaint.desaturate = _forwarder(_c.desaturate)
ifaint.desaturate_weighted = _forwarder(_c.desaturate_weighted)
ifaint.invert = _forwarder(_c.invert)
ifaint.next_frame = _forwarder(_c.next_frame)
ifaint.prev_frame = _forwarder(_c.prev_frame)
ifaint.redo = _forwarder(_c.redo)
ifaint.undo = _forwarder(_c.undo)
ifaint.zoom_default = _forwarder(_c.zoom_default)
ifaint.zoom_fit = _forwarder(_c.zoom_fit)
ifaint.zoom_in = _forwarder(_c.zoom_in)
ifaint.zoom_out = _forwarder(_c.zoom_out)

# Forwarding to the active image, supports arguments
ifaint.aa_line = _active(_c.aa_line)
ifaint.add_frame = _active(_c.add_frame)
ifaint.center = _active(_c.center)
ifaint.context_delete = _active(_c.context_delete)
ifaint.context_set_alpha = _active(_c.context_set_alpha)
ifaint.context_flip_horizontal = _active(_c.context_flip_horizontal)
ifaint.context_flip_vertical = _active(_c.context_flip_vertical)
ifaint.context_offset = _active(_c.context_offset)
ifaint.context_rotate_90CW = _active(_c.context_rotate_90CW)
ifaint.rotate = _active(_c.rotate)
ifaint.copy_rect = _active(_c.copy_rect)
ifaint.delete_objects = _active(_c.delete_objects)
ifaint.deselect = _active(_c.deselect)
ifaint.dwim = _active(_c.dwim)
ifaint.ellipse = _active(_c.ellipse)
ifaint.erase_but_color = _active(_c.erase_but_color)
ifaint.flatten = _active(_c.flatten)
ifaint.brightness_contrast = _active(_c.brightness_contrast)
ifaint.get_image_size = _active(_c.get_size)
ifaint.get_mouse_pos = _active(_c.get_mouse_pos)
ifaint.get_objects = _active(_c.get_objects)
ifaint.get_colors = _active(_c.get_colors)
ifaint.get_paint = _active(_c.get_paint)
ifaint.get_selected = _active(_c.get_selected)
ifaint.get_selection = _active(_c.get_selection)
ifaint.line = _active(_c.line)
ifaint.paste = _active(_c.paste)
ifaint.pixelize = _active(_c.pixelize)
ifaint.rect = _active(_c.rect)
ifaint.replace_color = _active(_c.replace_color)
ifaint.scroll_max_down = _active(_c.scroll_max_down)
ifaint.scroll_max_left = _active(_c.scroll_max_left)
ifaint.scroll_max_right = _active(_c.scroll_max_right)
ifaint.scroll_max_up = _active(_c.scroll_max_up)
ifaint.scroll_page_down = _active(_c.scroll_page_down)
ifaint.scroll_page_left = _active(_c.scroll_page_left)
ifaint.scroll_page_right = _active(_c.scroll_page_right)
ifaint.scroll_page_up = _active(_c.scroll_page_up)
ifaint.select = _active(_c.select)
ifaint.set_image_rect = _active(_c.set_rect)
ifaint.set_image_size = _active(_c.set_size)
ifaint.set_pixel = _active(_c.set_pixel)
ifaint.set_selection = _active(_c.set_selection)
ifaint.set_threshold = _active(_c.set_threshold)
ifaint.shrink_selection = _active(_c.shrink_selection)

def _scroll_top_left_all():
    for img in ifaint.images:
        img.scroll_max_left()
        img.scroll_max_up()
ifaint.scroll_top_left_all = _scroll_top_left_all

def _toggle_zoom_fit_all():
    """Toggles between zoom fit and zoom 1:1 for all images, zooming all
    depending on the current zoom of the active image"""
    active = ifaint.get_active_image()
    if active.get_zoom() == 1.0:
        ifaint.zoom_fit(ifaint.images)
    else:
        ifaint.zoom_default(ifaint.images)

bindc('e', ifaint.browse_to_active_file)
bindc('y', ifaint.dwim, mod.alt)
bindc('r', ifaint.prev_frame)
bindc('t', ifaint.next_frame)
bindk_global(key.f1, ifaint.dialog_help)
bindk_global(key.f8, ifaint.dialog_python_console)
bindk(key.end, ifaint.scroll_bottom_right, mod.alt)
bindk(key.end, ifaint.scroll_max_down, mod.ctrl)
bindk(key.end, ifaint.scroll_max_right)
bindk(key.home, ifaint.scroll_max_left)
bindk(key.home, ifaint.scroll_max_up, mod.ctrl)
bindk(key.home, ifaint.scroll_top_left, mod.alt)
bindk(key.home, ifaint.scroll_top_left_all, mod.alt|mod.ctrl)
bindk(key.pgdn, ifaint.scroll_page_down)
bindk(key.pgdn, ifaint.scroll_page_right, mod.ctrl)
bindk(key.pgup, ifaint.scroll_page_left, mod.ctrl)
bindk(key.pgup, ifaint.scroll_page_up)
bindk(key.num_minus, ifaint.zoom_out)
bindk(key.num_plus, ifaint.zoom_in)
bindk(key.backspace, ifaint.context_delete)
bindk(key.paragraph, ifaint.dialog_open_file, mod.ctrl)
bindk(key.asterisk, _toggle_zoom_fit_all, mod.ctrl)
bindk(key.arrow_left, NamedFunc("Offset Left",
    lambda: ifaint.context_offset(-1,0)))
bindk(key.arrow_right, NamedFunc("Offset Right",
    lambda: ifaint.context_offset(1,0)))
bindk(key.arrow_up, NamedFunc("Offset Up",
    lambda: ifaint.context_offset(0,-1)))
bindk(key.arrow_down, NamedFunc("Offset Down",
    lambda: ifaint.context_offset(0,1)))

def _scroll_traverse():
    """Scrolls through the image column by column, using
    scroll_page_down and scroll_page_right, eventually wrapping back
    to 0,0"""

    active = ifaint.get_active_image()
    max = active.get_max_scroll()
    current = active.get_scroll_pos()
    if current[1] >= max[1]:
        if current[0] >= max[0]:
            active.set_scroll_pos(0,0)
        else:
            active.set_scroll_pos(current[0], 0)
            active.scroll_page_right()
    else:
        active.scroll_page_down()
ifaint.scroll_traverse = _scroll_traverse

bindk(key.space, NamedFunc("Select or Traverse", lambda: (ifaint.select_top_object() if ifaint.get_layer() == 1 else ifaint.scroll_traverse())))

try:
    import faint.formatsvg as formatsvg
    ifaint.add_format(formatsvg.load, formatsvg.save, "Scalable Vector Graphics(*.svg)", "svg")
    del formatsvg

    import faint.formatsvgz as formatsvgz
    ifaint.add_format(formatsvgz.load, formatsvgz.save, "Zipped Scalable Vector Graphics(*.svgz)", "svgz")
    del formatsvgz
except Exception as e:
    ifaint.the_error = e

import faint.formatpdf as formatpdf
ifaint.add_format(formatpdf.load, formatpdf.save, "Portable Document Format (*.pdf)", "pdf")
del formatpdf

import collections

class ContextList(collections.Sequence):
    """Behaves like a list of objects, but is initialized on demand by
     calling a function"""

    def __init__(self, func, doc=None):
        self.func = func
        self.__doc__ = doc
    def __getitem__(self,key):
        return self.func()[key]
    def __len__(self):
        return len(self.func())
    def __repr__(self):
        return str(self.func())
    def __iter__(self):
        return self.func().__iter__()

def _list_frames():
    return ifaint.get_active_image().get_frames()
ifaint.list_frames = _list_frames

ifaint.frames = ContextList(ifaint.list_frames, "A list of frames in the active image.")
ifaint.images = ContextList(ifaint.list_images, "A list of all opened images.")
ifaint.objects = ContextList(ifaint.get_objects, "A list of all objects in the active image.")
ifaint.selected = ContextList(ifaint.get_selected, "A list of all selected objects in the active image.")

def anchor(images, anchors):
    """Expands the images so that all the anchors (one per image) end
    up at the same x,y position in all images.

    Useful for example when comparing diagrams, so that the content
    align in all images regardless of caption sizes

    """
    assert(len(images) == len(anchors))
    assert(len(images) > 1)
    max_x = anchors[0][0]
    max_y = anchors[0][1]
    for p in anchors:
        max_x = max(p[0], max_x)
        max_y = max(p[1], max_y)

    for img, anchor in zip(images, anchors):
        w, h = img.get_size()
        dx = anchor[0] - max_x
        dy = anchor[1] - max_y
        if (dx != 0 or dy != 0):
            img.set_rect(dx, dy, w - dx, h - dy)

_flagged = {}

def flagged():
    return copy.copy(_flagged)

def toggle_flag_pixel():
    """Adds the image and the current mouse position to a map of
    image-to-pixel. Indicates this with the set_point_overlay-function.

    If the mouse position is already flagged for the image, the flag
    is removed instead.

    The flagging can be used for to re-anchor images with anchor_flagged.

    """
    image = ifaint.get_active_image()
    pos = ifaint.get_mouse_pos()
    if image in _flagged and _flagged[image] == pos:
        del _flagged[image]
        image.clear_point_overlay()
    else:
        _flagged[image] = pos
        image.set_point_overlay(*pos)

def anchor_flagged():
    """Expands flagged images so that the flagged pixels are at the
    same x,y-position in all images.

    """
    anchor(_flagged.keys(), list(_flagged.values()))
    clear_flagged()

def clear_flagged():
    for image in _flagged:
        image.clear_point_overlay()
    _flagged.clear()

bindc('n', toggle_flag_pixel)

def help_class(c):
    print(c.__name__ + ":")
    for line in c.__doc__.split('\n'):
        print(" " + line)

    print()
    print(" Methods:")
    for name in sorted([item for item in c.__dict__ if not item.startswith("_")]):
        doc = c.__dict__[name].__doc__.split('\n')
        if len(doc) < 2:
            continue
        print("  " + doc[0], doc[1])
        print("    " + "\n    ".join(doc[2:]))

def help_method(m):
    print(m.__objclass__.__name__ + '.' + m.__name__ + ":")
    doc = m.__doc__.split('\n')
    for line in doc:
        print(" " + line)

def help_object(o):
    print(o.__doc__)

def help_intro():
    print("Faint Python Help")
    print()
    print("Use help(<name>) for help about named objects.")
    print("Example: help(Canvas)")

def help(item=None):
    """Python Help, customized for Faint"""
    if item == None:
        help_intro()
    elif item.__class__ == type:
        help_class(item)
    elif hasattr(item, '__objclass__'):
        help_method(item)
    else:
        help_object(item)

def reverse_frames():
    img = ifaint.get_active_image()
    n = len(img.get_frames())
    for i in range(n // 2):
        img.swap_frames(i, n - i - 1)

def next_left(obj):
    frame = get_object_frame(obj)
    by_x = sorted(frame.get_objects(), key=lambda o: o.pos()[0])
    i = by_x.index(obj)
    return by_x[i - 1]

def next_right(obj):
    frame = get_object_frame(obj)
    by_x = sorted(frame.get_objects(), key=lambda o: o.pos()[0])
    i = by_x.index(obj)
    return by_x[(i + 1) % len(by_x)]

def next_up(obj):
    frame = get_object_frame(obj)
    by_y = sorted(frame.get_objects(), key=lambda o: o.pos()[1])
    i = by_y.index(obj)
    return by_y[(i - 1)]

def next_down(obj):
    frame = get_object_frame(obj)
    by_y = sorted(frame.get_objects(), key=lambda o: o.pos()[1])
    i = by_y.index(obj)
    return by_y[(i + 1) % len(by_y)]

def next_behind(obj):
    frame = get_object_frame(obj)
    by_z = frame.get_objects()
    i = by_z.index(obj)
    return by_z[i - 1]

def next_ahead(obj):
    frame = get_object_frame(obj)
    by_z = frame.get_objects()
    i = by_z.index(obj)
    return by_z[(i + 1) % len(by_z)]

bindk(key.arrow_down, NamedFunc("Select next object down",
                                lambda: select(next_down(selected[0]))),
      mod.shift)

bindk(key.arrow_up, NamedFunc("Select next object up",
    lambda: select(next_up(selected[0]))),
    mod.shift)

bindk(key.arrow_left, NamedFunc("Select next object left",
    lambda: select(next_left(selected[0]))),
    mod.shift)


bindk(key.arrow_right, NamedFunc("Select next object right",
    lambda: select(next_right(selected[0]))),
    mod.shift)


bindk(key.arrow_up, NamedFunc("Select next behind",
    lambda: select(next_behind(selected[0]))),
    mod.alt)


def _select_next_front():
    if len(selected) == 0:
        if (len(objects) != 0):
            _select_bottom_object()
    else:
        select(next_ahead(selected[0]))

bindk(key.arrow_down, _select_next_front, mod.alt)

from ifaint import *

import faint.util
bindk(key.delete, faint.util.erase_selection, mod.ctrl)
