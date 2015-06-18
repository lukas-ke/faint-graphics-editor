def ns_svg(item_name):
    """Prepends the svg xml-namespace to the item name."""
    return '{http://www.w3.org/2000/svg}' + item_name


def ns_faint(item_name):
    """Prepends the faint xml-namespace to the item name."""
    return '{http://www.code.google.com/p/faint-graphics-editor}' + item_name


def ns_xlink(item_name):
    """Prepends the xlink xml-namespace to the item name."""
    return '{http://www.w3.org/1999/xlink}' + item_name

a = ns_svg('a')
circle = ns_svg('circle')
defs = ns_svg('defs')
ellipse= ns_svg('ellipse')
foreignObject = ns_svg('foreignObject')
g = ns_svg('g')
image = ns_svg('image')
line = ns_svg('line')
path = ns_svg('path')
polygon = ns_svg('polygon')
polyline = ns_svg('polyline')
rect = ns_svg('rect')
svg = ns_svg('svg')
switch = ns_svg('switch')
symbol = ns_svg('symbol')
text = ns_svg('text')
title = ns_svg('title')
use = ns_svg('use')
view = ns_svg('view')

class faint:
    calibration = ns_faint('calibration')

class xlink:
    href = ns_xlink('href')
