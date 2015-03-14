from ifaint import *
import os

# Fixme: Duplicates test-save-svg.py. Generalize. :)
# Fixme: Also totally fails.

def style_lw1(obj):
    obj.linewidth = 1

def style_lw2(obj):
    obj.linewidth = 2

def style_lw20(obj):
    obj.linewidth = 20

def style_none(obj):
    obj.fillstyle = 'none'
    obj.linewidth = 2

def style_border(obj):
    obj.fillstyle = 'b'
    obj.linewidth = 2

def style_fill(obj):
    obj.fillstyle = 'f'
    obj.linewidth = 2

def style_border_fill(obj):
    obj.fillstyle = 'bf'
    obj.linewidth = 2

# Row styles
def style_solid(obj):
    obj.linestyle = 's'

def style_solid_filled(obj):
    obj.linestyle = 's'
    obj.fillstyle = 'f'

def style_dashed(obj):
    obj.linestyle = 'ld'

def style_dashed_filled(obj):
    obj.linestyle = 'ld'
    obj.fillstyle = 'f'

# Column styles
def style_color(obj):
    obj.fg = (255,0,255)
    obj.bg = (0,255,255)

def style_gradient(obj):
    obj.fg = LinearGradient(0.0, (0.0,(255,0,0)), (1.0,(0,255,0)))
    obj.bg = LinearGradient(0.0, (0.0,(0,0,255)), (1.0,(255,0,0)))

def style_pattern(obj):
    p1 = Bitmap((10,10))
    p1.fill((0,0),(255,0,255))
    p1.line((0,0,9,9), (0,0,0))

    p2 = Bitmap((10,10))
    p2.fill((0,0),(0,255,255))
    p2.line((0,9,9,0), (0,0,0))

    obj.fg = Pattern(p1)
    obj.bg = Pattern(p2)

col_funcs = [(style_solid, "Solid"), (style_dashed, "Dashed")]

row_funcs = [(style_color, "Color"),
             (style_gradient, "Gradient"),
             (style_pattern, "Pattern")]

col_outer = [(style_border, "Border"),
             (style_fill, "Fill"),
             (style_border_fill, "Border+Fill"),
             (style_none, "None")]

row_outer = [(style_lw1, "1"),
             (style_lw2, "2"),
             (style_lw20, "20")]

CELL_WIDTH = 100
CELL_HEIGHT = 100
START_COL = 120
START_ROW = 40

HEADING_COL_2 = 40
HEADING_ROW_2 = 20

NUM_ROWS = len(row_funcs) * len(row_outer)
NUM_COLS = len(col_funcs) * len(col_outer)

IMAGE_WIDTH = CELL_WIDTH * NUM_COLS + START_COL
IMAGE_HEIGHT = CELL_HEIGHT * NUM_ROWS + START_ROW

img = Canvas(IMAGE_WIDTH,IMAGE_HEIGHT)

def apply_funcs(obj, row, col):
    cf = col_funcs[col % len(col_funcs)][0]
    rf = row_funcs[row % len(row_funcs)][0]
    outer_cf = col_outer[col // len(col_funcs)][0]
    outer_rf = row_outer[row // len(row_funcs)][0]
    cf(obj)
    rf(obj)
    outer_cf(obj)
    outer_rf(obj)
    return obj


def create_rect(img, row, col):
    r = img.Rect((START_COL + CELL_WIDTH * col + CELL_WIDTH / 4,
                  START_ROW + CELL_HEIGHT * row + CELL_HEIGHT / 4,
                  CELL_WIDTH / 2,
                  CELL_HEIGHT / 2))
    return apply_funcs(r, row, col)


def create_ellipse(img, row, col):
    e = img.Ellipse((START_COL + CELL_WIDTH * col + CELL_WIDTH / 4,
                  START_ROW + CELL_HEIGHT * row + CELL_HEIGHT / 4,
                  CELL_WIDTH / 2,
                  CELL_HEIGHT / 2))
    return apply_funcs(e, row, col)


def create_line(img, row, col):
    e = img.Line((START_COL + CELL_WIDTH * col + CELL_WIDTH / 4,
                  START_ROW + CELL_HEIGHT * row + CELL_HEIGHT / 4,
                  CELL_WIDTH / 2,
                  CELL_HEIGHT / 2))
    return apply_funcs(e, row, col)


def create_polygon(img, row, col):
    POLYGON_POINTS = [15.25, 0.25, 67.25, 31.5, 49.0, 85.25,
                  -0.5, 88.0, 71.5, 53.75, 5.25, 40.75, 35.0, 66.5]

    x0 = START_COL + CELL_WIDTH * col + 10
    y0 = START_ROW + CELL_HEIGHT * row + 5

    p = img.Polygon(POLYGON_POINTS)
    p.moveto(x0, y0)
    return apply_funcs(p, row, col)

# Lines separating inner and outer captions
img.Line((START_COL, HEADING_ROW_2, IMAGE_WIDTH, HEADING_ROW_2))
img.Line((HEADING_COL_2, START_ROW, HEADING_COL_2, IMAGE_HEIGHT))

# Vertical lines
for i in range(NUM_COLS):
    y0 = 0 if i % len(col_funcs) == 0 else HEADING_ROW_2
    img.Line((START_COL + CELL_WIDTH * i, y0,
              START_COL + CELL_WIDTH * i,IMAGE_HEIGHT))

# Horizontal lines
for i in range(NUM_ROWS):
    x0 = 0 if i % len(row_funcs) == 0 else HEADING_COL_2
    img.Line((x0,START_ROW + CELL_HEIGHT * i,
              IMAGE_WIDTH, START_ROW + CELL_HEIGHT * i))


# Column headings
for x, outer in enumerate(col_outer):
    offset = x * len(col_funcs) * CELL_WIDTH + 5
    w = CELL_WIDTH * len(col_funcs)
    text = img.Text((START_COL + CELL_WIDTH * len(col_funcs) * x, 5,
                     CELL_WIDTH * len(col_funcs), 20), outer[1])
    text.halign = 'center' # Fails on load
    text.text_render_style = 'pangolayout'

    for i, item in enumerate(col_funcs):
        img.Text((START_COL + CELL_WIDTH * i  + 5 +
                  offset, HEADING_ROW_2 + 5), item[1])


# Row headings
for x, outer in enumerate(row_outer):
    offset = x * len(row_funcs) * CELL_HEIGHT + 5

    text = img.Text((10, START_ROW + CELL_HEIGHT * len(row_funcs) * x, 10,
                     CELL_HEIGHT * len(row_funcs)),
                    outer[1])

    text.valign = 'middle'

    for i, item in enumerate(row_funcs):
        text = img.Text((HEADING_COL_2 + 10,
                  START_ROW + CELL_HEIGHT * i + offset,
                  100, CELL_HEIGHT),
                 item[1])
        text.valign = 'middle'

def for_each_cell(img, func):
    for row in range(NUM_ROWS):
        for col in range(NUM_COLS):
            func(img, row, col)

#for_each_cell(img, create_polygon)
for_each_cell(img, create_ellipse)
#for_each_cell(img, create_line)


# Save and load again
file_name = os.path.join(os.getcwd(), 'out', 'test-save-pdf.pdf')
img.save_backup(file_name)
img2 = Canvas(file_name)
