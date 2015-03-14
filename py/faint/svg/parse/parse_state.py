import ifaint
from . parse_util import Matrix
import faint.svg.svg_re as svg_re
import re
from . parse_util import (
    apply_transforms,
    ABSOLUTE_UNIT_FACTORS,
    parse_color,
    parse_color_noref,
)

from faint.svg.util import (
    deg2rad
)

from faint.svg.ifaint_util import (
    mul_matrix_tri,
    remove_stroke,
    remove_fill,
    to_faint_cap_str)


class ParseState:
    """The state when arriving at a node. Contains the inheritable
    settings from "above", as well as the current transformation matrix
    (ctm) and the frame-props.

    """

    def __init__(self, frame_props,
                 ctm=None,
                 settings=None,
                 ids=None,
                 currentColor=None,
                 containerSize=None,
                 system_language="en"):

        self.props = frame_props
        self.ctm = Matrix.identity() if ctm is None else ctm
        self.settings = (node_default_settings() if settings is None
                         else settings)
        self.ids = {} if ids is None else ids
        self.currentColor = ((255, 0, 0) if currentColor is None
                             else currentColor)
        self.containerSize = ((0.0, 0.0)
                              if containerSize is None else containerSize)

        # ISO 639-1 language code
        self.system_language = system_language


    def modified(self, ctm=None, settings=None, currentColor=None, containerSize=None):
        """Returns a new ParseState for recursing, with optionally updated
        transformation matrix or settings. The props is unchanged.

        """
        if ctm is None:
            ctm = self.ctm
        if settings is None:
            settings = self.settings
        if currentColor is None:
            currentColor = self.currentColor
        if containerSize is None:
            containerSize = self.containerSize
        return ParseState(self.props, ctm, settings, self.ids,
                             currentColor, containerSize, self.system_language)


    def transform_object(self, object_id):
        """Transform the object with object_id in props with the CTM."""

        tri = self.props.get_obj_tri(object_id)
        tri = mul_matrix_tri(self.ctm, tri)
        self.props.set_obj_tri(object_id, tri)


    def transform_tri(self, tri):
        """Return the tri transformed by the CTM"""
        return mul_matrix_tri(self.ctm, tri)


    def updated(self, node):
        """Return a new ParseState updated by the node attributes (e.g.
        transform, settings etc.)

        """
        assert self.ctm is not None
        transforms = parse_transform_list(node.get('transform', ''))
        ctm = apply_transforms(transforms, self.ctm)


        cc = node.get('color')
        if cc is not None:
            cc = parse_color_noref(cc, 1.0, self)

        # Fixme: Ugly. Create the new state instead
        currentCurrentColor = self.currentColor
        if cc is not None:
            self.currentColor = cc
        settings = self._updated_settings(node, ctm)
        self.currentColor = currentCurrentColor

        return self.modified(ctm=ctm, settings=settings, currentColor=cc)


    def _updated_settings(self, node, ctm):
        # Fixme: Move all stuff from parse_style_dict here.
        # Fixme: Traverse the keys instead
        # Fixme: Handle 'inherit'
        settings = ifaint.Settings()
        settings.update_all(self.settings)
        settings.fg = self.settings.fg
        settings.bg = self.settings.bg
        attributes = get_style_dict(node.get('style', ''))
        attributes.update(node.attrib)

        settings.fillstyle = self.settings.fillstyle
        #settings.update_all(self.settings)

        stroke_opacity = attributes.get("stroke-opacity", "1.0")
        fill_opacity = attributes.get("fill-opacity", "1.0")
        stroke = attributes.get("stroke")
        fill = attributes.get("fill")
        fillstyle_to_settings(settings, stroke, fill, stroke_opacity, fill_opacity, self)

        stroke_width = attributes.get('stroke-width')

        if stroke_width == "inherit":
            pass
        elif stroke_width is not None:
            # Fixme: Should this be the new state rather than self?
            sw1 = svg_length_attr(stroke_width, self)
            # Fixme: Using the new ctm, verify.
            # Fixme: using .a is simplistic
            sw1 *= ctm.a
            try:
                settings.linewidth = sw1
            except ValueError as e:
                # Todo: Allow larger stroke width in Faint
                # also, check how to handle negative.
                self.props.add_warning(str(e))

        stroke_dasharray = attributes.get('stroke-dasharray', None)
        if stroke_dasharray is not None:
            if stroke_dasharray == 'none':
                # Fixme: Verify "none"
                settings.linestyle = 's'
            else:
                # Fixme: actually use the dash-array
                settings.linestyle = 'ld'

        stroke_linejoin = attributes.get('stroke-linejoin', None)
        if stroke_linejoin is not None and stroke_linejoin != 'inherit':
            # Fixme: settings.join probably not 100% aligned with
            # svg join
            settings.join = stroke_linejoin

        stroke_linecap = attributes.get('stroke-linecap', None)
        if stroke_linecap is not None:
            settings.cap = to_faint_cap_str(stroke_linecap)

        font_size = attributes.get('font-size', None)
        if font_size is not None:
            # Fixme: What other names are there?
            if font_size == 'medium':
                settings.fontsize = 12.0 # Fixme
            elif font_size == 'inherit':
                # TODO: Do what?
                pass
            else:
                # Fixme: Terrible
                settings.fontsize = svg_length_attr(font_size, self)

        font_family = attributes.get("font-family", None)
        if font_family is not None:
            # Fixme: face vs family eh
            settings.font = font_family

        font_style = attributes.get("font-style", None)
        if font_style is not None:
            # Fixme: Extend
            settings.fontitalic = font_style == "italic"

        font_weight = attributes.get('font-weight', None)
        if font_weight is not None:
            settings.fontbold = font_weight == 'bold'

        parse_marker_attr(node, settings)
        return settings


    def add_warning(self, text):
        """Add a load warning to the contained props."""
        self.props.add_warning(text)


    def add_ref_item(self, ref_id, obj):
        """Add an item accessible by id (item is a tuple, id to object)"""
        self.ids[ref_id] = obj


def add_fill(settings, fill, fill_opacity, state):
    """Adds the specified fill-style to the settings object, preserving
    border.

    """
    if settings.fillstyle == 'border':
        settings.fillstyle = 'border+fill'
        settings.bg = parse_color(fill, fill_opacity, state)
    else:
        settings.fillstyle = 'fill'
        settings.fg = parse_color(fill, fill_opacity, state)


def add_stroke(settings, stroke, stroke_opacity, state):
    """Adds the specified stroke-style to the settings object, preserving
    fill

    """
    if settings.fillstyle == 'fill':
        settings.fillstyle = 'border+fill'
        settings.bg = settings.fg
        settings.fg = parse_color(stroke, stroke_opacity, state)
    else:
        settings.fillstyle = 'border'
        settings.fg = parse_color(stroke, stroke_opacity, state)


def parse_marker_attr(node, settings):
    """Parses the node's SVG 'marker-start', 'marker-end' attributes

    """
    arrowhead_str = node.get('marker-end')
    arrowtail_str = node.get('marker-start')
    # Fixme: actually refer to marked structure
    arrowhead = (arrowhead_str == 'url(#Arrowhead)')
    arrowtail = (arrowtail_str == 'url(#Arrowtail)')

    if arrowhead and arrowtail:
        settings.arrow = 'both'
    elif arrowtail:
        settings.arrow = 'back'
    elif arrowhead:
        settings.arrow = 'front'
    else:
        settings.arrow = 'none'


def fillstyle_to_settings(settings, stroke, fill, stroke_opacity, fill_opacity, state):
    """Converts from SVG stroke and fill to the combined faint fillstyle +
    fgcol, bgcol.

    """

    # Fixme: Simplify
    if stroke == None and fill != None:
        if fill == "none":
            remove_fill(settings)
        else:
            add_fill(settings, fill, fill_opacity, state)
        return
    elif stroke != None and fill == None:
        if stroke == "none":
            remove_stroke(settings)
        else:
            add_stroke(settings, stroke, stroke_opacity, state)
        return

    elif stroke != None and fill != None:
        if stroke == "none" and fill == "none":
            settings.fillstyle = 'none'
            return

        elif stroke == "none" and fill != "none":
            settings.fillstyle = 'fill'
            settings.fg = parse_color(fill, fill_opacity, state)
            return

        elif stroke != "none" and fill == "none":
            settings.fillstyle = 'border'
            settings.fg = parse_color(stroke, stroke_opacity, state)
            return

        elif stroke != "none" and fill != "none":
            settings.fillstyle = 'border+fill'
            settings.fg = parse_color(stroke, stroke_opacity, state)
            settings.bg = parse_color(fill, fill_opacity, state)
            return


def get_style_dict(style):
    """Parses an SVG style attribute string, returning it as a key/value
    dictionary.

    """
    # Fixme: Review, from old
    style_items = style.split(";")
    style_dict = {}
    for item in style_items:
        if len(item) > 0:
            key, value = item.split(":")
            key, value = key.strip(), value.strip()
            style_dict[key] = value
    return style_dict


def node_default_settings():
    """Returns the initial default Settings."""
    # Fixme: Review, from old
    settings = ifaint.Settings()
    settings.linewidth = 1.0
    settings.cap = 'flat'
    settings.fg = (0, 0, 0)
    settings.bg = (0, 0, 0)
    settings.fillstyle = 'fill'
    return settings


def parse_transform(s):
    """Parses an entry in a transform-list."""
    def _parse_args(s):
        assert s.startswith("(")
        assert s.endswith(")")
        # Fixme: parse number (i.e. incl. Ee etc)

        str_args = [arg for arg in s.replace(" ", ",")[1:-1].split(",") if len(arg) != 0]
        return [float(arg) for arg in str_args]

    op, args = s.split("(")
    args = _parse_args('(' + args)

    if op == "skewX":
        return Matrix.skewX(deg2rad(*args))
    elif op == "skewY":
        return Matrix.skewY(deg2rad(*args))
    elif op == "rotate":
        rad = deg2rad(args[0])
        pivot = args[1:] if len(args) == 3 else None
        return Matrix.rotation(rad, pivot)
    elif op == "translate":
        x = args[0]
        y = args[1] if len(args) == 2 else 0
        return Matrix.translation(x,y)
    elif op == "matrix":
        return Matrix(*args)
    elif op == "scale":
        sx = args[0]
        sy = args[1] if len(args) == 2 else sx
        return Matrix.scale(sx, sy)
    else:
        raise svg_error("Unsupported transform: %s" % op)


def parse_transform_list(s):
    """Parses an SVG transform attribute"""
    transform_list = re.findall(svg_re.transform_list, s)
    return [parse_transform(tr) for tr in transform_list]


def svg_length_attr_dumb(value_str, props, full_span):
    """Parses an svg length-attribute from the value_str. full_span is
    used as reference for percentages."""

    m = re.match(svg_re.length_attr, value_str)
    if m is None:
        raise SvgLengthError(value_str)

    value, unit = m.groups()
    if unit == "%":
        # Fixme: More work required.
        if full_span.__class__ == float:
            return (full_span * 100) / float(value)
        else:
            return (full_span[0] * 100) / float(value)
    elif unit in ABSOLUTE_UNIT_FACTORS:
        return float(value) * ABSOLUTE_UNIT_FACTORS[unit]
    elif unit in ('em','ex'):
        # Fixme: Implement.
        props.add_warning("Unsupported unit: %s" % unit)
        return float(value)
    else:
        props.add_warning("Invalid unit: %s" % unit)
        return float(value)


def svg_length_attr(value_str, state):
    """Parses an svg length attribute."""
    return svg_length_attr_dumb(value_str, state.props, state.containerSize)
