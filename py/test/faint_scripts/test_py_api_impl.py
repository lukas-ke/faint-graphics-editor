#!/usr/bin/env python3
# -*- coding: utf-8 -*-

from ifaint import *
import os

# Fixme: Migrate as much as possible to tests/py_ext_tests

def test(fail_if):
    # FaintApp
    fail_if(app.gridcolor != (100,100,255,150))
    fail_if(app.griddashed != True)
    fail_if(not app.get_checkered_transparency_indicator())
    app.set_transparency_indicator((255,0,255))
    fail_if(app.get_checkered_transparency_indicator())
    # Fixme:  app.open_files

    # Note: Bitmap tests removed in favor of test_bitmap.py using
    # extension module

    # Canvas
    canvas = get_active_image()
    fail_if(canvas.get_paint((0,0)) != (255,255,255,255))
    canvas.center(0,0)
    fail_if(canvas.get_mouse_pos() != (0,0))
    fail_if(canvas.get_point_overlay() is not None)
    canvas.set_point_overlay(1,2)
    fail_if(canvas.get_point_overlay() != (1,2))
    canvas.clear_point_overlay()
    fail_if(canvas.get_point_overlay() is not None)
    canvas.rect((15,20,30,40))
    context_crop(canvas)
    fail_if(canvas.get_size() != (30,40))
    fail_if(canvas.get_paint((0,0)) != (0,0,0,255))
    fail_if(canvas.get_paint((29,39)) != (0,0,0,255))
    fail_if(canvas.get_paint((1,1)) != (255,255,255,255))
    fail_if(len(canvas.get_objects()) != 0)
    canvas.set_rect((0,0,640,480))
    fail_if(canvas.get_size() != (640,480))
    canvas.undo()
    fail_if(canvas.get_size() != (30,40))
    canvas.redo()
    fail_if(canvas.get_size() != (640,480))
    fail_if(canvas.get_paint(0,0) != (0,0,0,255))
    fail_if(canvas.get_paint(29,39) != (0,0,0,255))
    fail_if(canvas.get_paint(30,40) != (255,255,255,255))

    fail_if(canvas.get_zoom() != 1.0)
    e = canvas.Ellipse((10,20,30,40))
    fail_if(str(e) != 'Ellipse')
    select(objects)
    fail_if(list(selected) != [e])
    select([])
    fail_if(list(selected) != [])
    select(objects[:])
    fail_if(list(selected) != [e])
    canvas.flatten(e)
    fail_if(len(selected) != 0)
    undo()
    fail_if(len(selected) != 0) # No object selection undo
    select(objects)
    fail_if(list(selected) != [e])

    s = get_settings()

    # Defaults
    fail_if(s.fg != (0,0,0,255))
    fail_if(s.bg != (255,255,255,255))
    s.fg = 255,0,0
    s.bg = 0,0,255
    update_settings(s)
    fail_if(get_fg() != s.fg)
    fail_if(get_bg() != s.bg)

    text = Text((10,10), "Ödla")
    fail_if(text.get_text_evaluated() != "Ödla")
    g = Group(e, text)
    fail_if(g.num_objs() != 2)

    text2 = Text((100,10), "Jürgen")
    fail_if(text2.get_text_evaluated() != "Jürgen")

    r = Rect((0,0,200,200))
    r.name = "MyRect"
    fail_if(r.name != "MyRect")
    undo()
    fail_if(r.name is not None)
    redo()
    fail_if(r.name != "MyRect")
