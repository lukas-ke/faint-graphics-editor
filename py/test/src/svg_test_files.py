#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# List of files from the SVG test suite that Faint has
# a chance of parsing somewhat.
# http://www.w3.org/Graphics/SVG/WG/wiki/Test_Suite_Overview
TESTABLE_FILES = [
    # Color properties
    # ----------------------------------------
    'color-prop-01-b.svg',
    'color-prop-02-f.svg',
    'color-prop-03-t.svg',
    'color-prop-04-t.svg',
    'color-prop-05-t.svg',

    # Conforming viewers
    # ----------------------------------------
    'conform-viewers-01-t.svgz',
    'conform-viewers-02-f.svg', # Fails: Embedded zip
    'conform-viewers-03-f.svg', # Fails: keyerror script
    'coords-coord-01-t.svg', # Ok, slightly offset
    'coords-coord-02-t.svg', # Wrong color for edge dots, otherwise OK
    'coords-trans-01-b.svg', # Horizontal rectangles totally broken
    'coords-trans-02-t.svg',
    'coords-trans-03-t.svg', # Line thickness not right
    'coords-trans-04-t.svg', # Lines gone
    'coords-trans-05-t.svg', # Lines gone
    'coords-trans-06-t.svg', # Lines gone
    'coords-trans-07-t.svg',
    'coords-trans-08-t.svg',
    'coords-trans-09-t.svg',
    'coords-trans-10-t.svg',
    'coords-trans-11-f.svg', # Ellipses, text too small
    'coords-trans-12-f.svg', # Lines at wrong place
    'coords-trans-13-f.svg', # Text destroyed
    'coords-trans-14-f.svg', # Lines at wrong place and such
    'coords-transformattr-01-f.svg',
    'coords-transformattr-02-f.svg',
    'coords-transformattr-03-f.svg',
    'coords-transformattr-04-f.svg',
    'coords-transformattr-05-f.svg',
    'coords-units-01-b.svg', # Missing required positional argument
                             # (parse lg node)
    'coords-units-02-b.svg', # Some OK, some wrong
    'coords-units-03-b.svg,' # Lines missing, text wrong
    'coords-viewattr-01-b.svg', # All smileys fail
    'coords-viewattr-02-b.svg', # All smileys are stretched
    'coords-viewattr-03-b.svg', # Nothing shown
    'coords-viewattr-04-f.svg', # Ignored image with type None

    # Filters
    # ----------------------------------------
    #'filters-background-01-f.svg',
    #'filters-blend-01-b.svg',
    #'filters-color-01-b.svg',
    #'filters-color-02-b.svg',
    #'filters-composite-02-b.svg',
    #'filters-composite-03-f.svg',
    #'filters-composite-04-f.svg',
    #'filters-composite-05-f.svg',
    #'filters-comptran-01-b.svg',
    #'filters-conv-01-f.svg',
    #'filters-conv-02-f.svg',
    #'filters-conv-03-f.svg',
    #'filters-conv-04-f.svg',
    #'filters-conv-05-f.svg',
    #'filters-diffuse-01-f.svg',
    #'filters-displace-01-f.svg',
    #'filters-displace-02-f.svg',
    #'filters-example-01-b.svg',
    #'filters-felem-01-b.svg',
    #'filters-felem-02-f.svg',
    #'filters-gauss-01-b.svg',
    #'filters-gauss-02-f.svg',
    #'filters-gauss-03-f.svg',
    #'filters-image-01-b.svg',
    #'filters-image-02-b.svg',
    #'filters-image-03-f.svg',
    #'filters-image-04-f.svg',
    #'filters-image-05-f.svg',
    #'filters-light-01-f.svg',
    #'filters-light-02-f.svg',
    #'filters-light-03-f.svg',
    #'filters-light-04-f.svg',
    #'filters-light-05-f.svg',
    #'filters-morph-01-f.svg',
    #'filters-offset-01-b.svg',
    #'filters-offset-02-b.svg',
    #'filters-overview-01-b.svg',
    #'filters-overview-02-b.svg',
    #'filters-overview-03-b.svg',
    #'filters-specular-01-f.svg',
    #'filters-tile-01-b.svg',
    #'filters-turb-01-f.svg',
    #'filters-turb-02-f.svg',

    # Fonts
    # ----------------------------------------
    #'fonts-desc-01-t.svg',
    #'fonts-desc-02-t.svg',
    #'fonts-desc-03-t.svg',
    #'fonts-desc-04-t.svg',
    #'fonts-desc-05-t.svg',
    #'fonts-elem-01-t.svg',
    #'fonts-elem-02-t.svg',
    #'fonts-elem-03-b.svg',
    #'fonts-elem-04-b.svg',
    #'fonts-elem-05-t.svg',
    #'fonts-elem-06-t.svg',
    #'fonts-elem-07-b.svg',
    #'fonts-glyph-02-t.svg',
    #'fonts-glyph-03-t.svg',
    #'fonts-glyph-04-t.svg',
    #'fonts-kern-01-t.svg',
    #'fonts-overview-201-t.svg',

    # Markers on zero length path and lines
    # ----------------------------------------
    'imp-path-01-f.svg', # Nothing drawn

    # Linking
    # ----------------------------------------
    #'linking-a-01-b.svg',
    #'linking-a-03-b.svg',
    #'linking-a-04-t.svg',
    #'linking-a-05-t.svg',
    #'linking-a-07-t.svg',
    #'linking-a-08-t.svg',
    #'linking-a-09-b.svg',
    #'linking-a-10-f.svg',
    #'linking-frag-01-f.svg',
    #'linking-uri-01-b.svg',
    #'linking-uri-02-b.svg',
    #'linking-uri-03-t.svg',

    # Masking
    # ----------------------------------------
    #'masking-filter-01-f.svg',
    #'masking-intro-01-f.svg',
    #'masking-mask-01-b.svg',
    #'masking-mask-02-f.svg',
    #'masking-opacity-01-b.svg',
    #'masking-path-01-b.svg',
    #'masking-path-02-b.svg',
    #'masking-path-03-b.svg',
    #'masking-path-04-b.svg',
    #'masking-path-05-f.svg',
    #'masking-path-06-b.svg',
    #'masking-path-07-b.svg',
    #'masking-path-08-b.svg',
    #'masking-path-09-b.svg',
    #'masking-path-10-b.svg',
    #'masking-path-11-b.svg',
    #'masking-path-12-f.svg',
    #'masking-path-13-f.svg',
    #'masking-path-14-f.svg',

    # Metadata
    # ----------------------------------------
    #'metadata-example-01-t.svg',

    # Painting
    # ----------------------------------------
    'painting-control-01-f.svg',
    'painting-control-02-f.svg',
    'painting-control-03-f.svg',
    'painting-control-04-f.svg',
    'painting-control-05-f.svg',
    'painting-control-06-f.svg',
    'painting-fill-01-t.svg',
    'painting-fill-02-t.svg',
    'painting-fill-03-t.svg', # Fails: Both use nonzero winding
    'painting-fill-04-t.svg', # Fails: Incorrect stroke/fill inheritance
    'painting-fill-05-b.svg', # Glitchy, (error in blending, not color parsing)
    'painting-marker-01-f.svg', # Faint doesn't parse markers
    'painting-marker-02-f.svg',
    'painting-marker-03-f.svg',
    'painting-marker-04-f.svg',
    'painting-marker-05-f.svg', # Unsupported unit: em
    'painting-marker-06-f.svg',
    'painting-marker-07-f.svg', # Group in def fails: Passed id-to-etree node
    'painting-marker-properties-01-f.svg',
    'painting-render-01-b.svg',
    'painting-render-02-b.svg',
    'painting-stroke-01-t.svg',
    'painting-stroke-02-t.svg',
    'painting-stroke-03-t.svg', # Line cap wrong. Miterlimit??
    'painting-stroke-04-t.svg', # Dash-array
    'painting-stroke-05-t.svg', # Close, but I render 0-thickness and offset
    'painting-stroke-06-t.svg',
    'painting-stroke-07-t.svg', # Miter limit
    'painting-stroke-08-t.svg', # Almost, but not perfect lineup.
    'painting-stroke-09-t.svg',
    'painting-stroke-10-t.svg', # Zero length sub-path

    #Paths data
    #----------------------------------------
    'paths-data-01-t.svg', # Some paths broken.
    'paths-data-02-t.svg', # Nothing drawn. Quadratic curves.
    'paths-data-03-f.svg', # Some arcs, totally wrong
    'paths-data-04-t.svg', # Sub-paths
    'paths-data-05-t.svg', # Sub-paths
    'paths-data-06-t.svg',
    'paths-data-07-t.svg',
    'paths-data-08-t.svg',
    'paths-data-09-t.svg', # Sub paths
    'paths-data-10-t.svg', # Bevel, miter join
    'paths-data-12-t.svg',
    'paths-data-13-t.svg',
    'paths-data-14-t.svg', # Sub paths
    'paths-data-15-t.svg',
    'paths-data-16-t.svg',
    'paths-data-17-f.svg',
    'paths-data-18-f.svg',
    'paths-data-19-f.svg',
    'paths-data-20-f.svg', # Elliptical arc, something goes wrong

    #pservers
    #----------------------------------------
    #'pservers-grad-01-b.svg',
    #'pservers-grad-02-b.svg',
    #'pservers-grad-03-b.svg',
    #'pservers-grad-04-b.svg',
    #'pservers-grad-05-b.svg',
    #'pservers-grad-06-b.svg',
    #'pservers-grad-07-b.svg',
    #'pservers-grad-08-b.svg',
    #'pservers-grad-09-b.svg',
    #'pservers-grad-10-b.svg',
    #'pservers-grad-11-b.svg',
    #'pservers-grad-12-b.svg',
    #'pservers-grad-13-b.svg',
    #'pservers-grad-14-b.svg',
    #'pservers-grad-15-b.svg',
    #'pservers-grad-16-b.svg',
    #'pservers-grad-17-b.svg',
    #'pservers-grad-18-b.svg',
    #'pservers-grad-20-b.svg',
    #'pservers-grad-21-b.svg',
    #'pservers-grad-22-b.svg',
    #'pservers-grad-23-f.svg',
    #'pservers-grad-24-f.svg',
    #'pservers-grad-stops-01-f.svg',
    #'pservers-pattern-01-b.svg',
    #'pservers-pattern-02-f.svg',
    #'pservers-pattern-03-f.svg',
    #'pservers-pattern-04-f.svg',
    #'pservers-pattern-05-f.svg',
    #'pservers-pattern-06-f.svg',
    #'pservers-pattern-07-f.svg',
    #'pservers-pattern-08-f.svg',
    #'pservers-pattern-09-f.svg',

    #Render elements
    #----------------------------------------
    'render-elems-01-t.svg',
    'render-elems-02-t.svg',
    'render-elems-03-t.svg',
    'render-elems-06-t.svg', # Argument out of range
    'render-elems-07-t.svg', # Argument out of range
    'render-elems-08-t.svg', # Argument out of range

    # Render groups
    # ----------------------------------------
    'render-groups-01-b.svg',
    'render-groups-03-t.svg',

    # Shapes
    # ----------------------------------------
    'shapes-circle-01-t.svg', # OK imo
    'shapes-circle-02-t.svg', # Type error
    'shapes-ellipse-01-t.svg', # OK imo
    'shapes-ellipse-02-t.svg', # Type error
    'shapes-ellipse-03-f.svg',
    'shapes-grammar-01-f.svg', # ValueError
    'shapes-intro-01-t.svg', # Shapes visible. ..Though that's the Faint way..
    'shapes-intro-02-f.svg', # Red visible
    'shapes-line-01-t.svg',
    'shapes-line-02-f.svg',
    'shapes-polygon-01-t.svg',
    'shapes-polygon-02-t.svg', # Black fill etc?
    'shapes-polygon-03-t.svg', # Number of parameters is not even
    'shapes-polyline-01-t.svg', # Filled polyline not filled.
    'shapes-polyline-02-t.svg', # Filled polyline not filled.
    'shapes-rect-01-t.svg', # Rounded rectangle missing
    'shapes-rect-02-t.svg', # TypeError
    'shapes-rect-03-t.svg', # TypeError
    'shapes-rect-04-f.svg', # Rounded edges
    'shapes-rect-05-f.svg', # A bit of orange is visible here and there
    'shapes-rect-06-f.svg',# Rounded rectangles (pass due to poor wording ;)
    'shapes-rect-07-f.svg', # Again rounded

    # Structural
    # ----------------------------------------
    'struct-cond-01-t.svg',
    'struct-cond-02-t.svg',
    'struct-cond-03-t.svg',
    'struct-cond-overview-02-f.svg', # <use>
    'struct-cond-overview-03-f.svg', # Ok, but minor pass
    'struct-cond-overview-04-f.svg',
    'struct-cond-overview-05-f.svg',
    'struct-defs-01-t.svg',
    'struct-frag-01-t.svg',
    'struct-frag-02-t.svg', # Aspect ratio
    'struct-frag-03-t.svg', # Aspect ratio
    'struct-frag-04-t.svg', # OK:ish..., though img size?
    'struct-frag-05-t.svg',
    'struct-frag-06-t.svg', # OK, but how? "entity references??"

    # Structure: Group
    # ----------------------------------------
    'struct-group-01-t.svg', # OK, but spacing
    'struct-group-02-b.svg', # Nothing
    'struct-group-03-t.svg', # ValueError, must support inherit

    # Structure: Image
    # ----------------------------------------
    'struct-image-01-t.svg', # Unsupported image type
    'struct-image-02-b.svg', # Nothing rendered (thank god)
    'struct-image-03-t.svg',
    'struct-image-04-t.svg',
    'struct-image-05-b.svg',
    'struct-image-06-t.svg',
    'struct-image-07-t.svg',
    'struct-image-08-t.svg',
    'struct-image-09-t.svg',
    'struct-image-10-t.svg',
    'struct-image-11-b.svg',
    'struct-image-12-b.svg',
    'struct-image-13-f.svg',
    'struct-image-14-f.svg',
    'struct-image-15-f.svg',
    'struct-image-16-f.svg',
    'struct-image-17-b.svg',
    'struct-image-18-f.svg',
    'struct-image-19-f.svg',

    # Structure: SVG
    # ----------------------------------------
    'struct-svg-01-f.svg',
    'struct-svg-02-f.svg', # Also png wrong (ok in firefox)
    'struct-svg-03-f.svg',

    # Structure: Symbol
    # ----------------------------------------
    'struct-symbol-01-b.svg', # Nothing rendered

    # Structure: Use
    # ----------------------------------------
    'struct-use-01-t.svg',
    'struct-use-03-t.svg',
    'struct-use-04-b.svg',
    'struct-use-05-b.svg',
    'struct-use-06-b.svg',
    'struct-use-07-b.svg',
    'struct-use-08-b.svg',
    'struct-use-09-b.svg',
    'struct-use-10-f.svg',
    'struct-use-11-f.svg',
    'struct-use-12-f.svg',
    'struct-use-13-f.svg',
    'struct-use-14-f.svg',
    'struct-use-15-f.svg',

    # Text
    # ----------------------------------------
    'text-align-01-b.svg',
    # 'text-align-02-b.svg',
    # 'text-align-03-b.svg',
    # 'text-align-04-b.svg',
    # 'text-align-05-b.svg',
    # 'text-align-06-b.svg',
    # 'text-align-07-t.svg',
    # 'text-align-08-b.svg',
    # 'text-altglyph-01-b.svg',
    # 'text-altglyph-02-b.svg',
    # 'text-altglyph-03-b.svg',
    # 'text-bidi-01-t.svg',
    # 'text-deco-01-b.svg',
    # 'text-fonts-01-t.svg',
    # 'text-fonts-02-t.svg',
    # 'text-fonts-03-t.svg',
    # 'text-fonts-04-t.svg',
    # 'text-fonts-05-f.svg',
    # 'text-fonts-202-t.svg',
    # 'text-fonts-203-t.svg',
    # 'text-fonts-204-t.svg',
    # 'text-intro-01-t.svg',
    # 'text-intro-02-b.svg',
    # 'text-intro-03-b.svg',
    # 'text-intro-04-t.svg',
    # 'text-intro-05-t.svg',
    # 'text-intro-06-t.svg',
    # 'text-intro-07-t.svg',
    # 'text-intro-09-b.svg',
    # 'text-intro-10-f.svg',
    # 'text-intro-11-t.svg',
    # 'text-intro-12-t.svg',
    # 'text-path-01-b.svg',
    # 'text-path-02-b.svg',
    # 'text-spacing-01-b.svg',
    # 'text-text-01-b.svg',
    # 'text-text-03-b.svg',
    # 'text-text-04-t.svg',
    # 'text-text-05-t.svg',
    # 'text-text-06-t.svg',
    # 'text-text-07-t.svg',
    # 'text-text-08-b.svg',
    # 'text-text-09-t.svg',
    # 'text-text-10-t.svg',
    # 'text-text-11-t.svg',
    # 'text-text-12-t.svg',
    # 'text-tref-01-b.svg',
    # 'text-tref-02-b.svg',
    # 'text-tref-03-b.svg',
    # 'text-tselect-01-b.svg',
    # 'text-tselect-02-f.svg',
    # 'text-tselect-03-f.svg',
    # 'text-tspan-01-b.svg',
    # 'text-tspan-02-b.svg',
    # 'text-ws-01-t.svg',
    # 'text-ws-02-t.svg',
    # 'text-ws-03-t.svg',

    # Types (basic)
    # ----------------------------------------
    'types-basic-01-f.svg',
    'types-basic-02-f.svg', # css, though should upper case PX be supported?
]
