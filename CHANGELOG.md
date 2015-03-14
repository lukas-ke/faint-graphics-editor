# Change Log
All notable changes to Faint graphics editor are documented in this file.

## [Unreleased][unreleased]
Long overdue release!  
**Highlights**:

- hotspot, calibration and tape-measure tools.
- expression support in text objects  
- improved SVG handling.
- more properties, less methods in Python API.

### Added
- Added hot-spot tool, for setting the hot spot in a frame. This was
  previously available only via `frame.set_hotspot` Python function.

- Added calibration tool for defining image measurements. This
  can be used with text-expressions to show the length or area of objects.

- Added tape-measure tool for showing image measurements.

- Added support for expressions in text objects. These can be used
  to specify symbols (e.g. `\pi` or `\poop`), or to retrieve the area of an object (`area(rectangle1, mm2)`). 

- An Object's name can be specified by selecting an object and pressing enter.
  This name can be used to refer to the object in text expressions.

- Added rounded rectangles.

- Added `--arg` command line option which stores the specified value as a string
  in ifaint.cmd_arg, for use with command line scripts.

- Added Ctrl+T for transposing characters during text entry.

- Added support for typing expressions in text objects, which are
automatically converted like: "The rectangle is \width(rect1,mm) mm
wide." becomes e.g. "The rectangle is 22 mm wide".

- Added support for specifying image measurements in various units
for use in text expressions.

- Added support for specifying character contants in text objects, which are
shown as symbols when the text objects are not being edited, for example
\pi for a pi-symbol,
\deg for degree
\sq for superscript 2
Also \u(hex) allows specifying a unicode symbol by hex value.

- Added render-style option for text objects to render either as a
cairo path or pango layout. The pango layout rendering is "wobbly"
if the text is rotated on windows, but often looks better than
rendering as a cairo-path when not rotated.
Only available via Python,
obj.text_render_style = 'pangolayout' or 'cairopath'

- Added checkbox for nearest neighbour scaling to resize dialog for
image and raster selection.

- Added dialog for adjusting alpha transparency.

- Flood fill supports gradient as fill color.

- Brush tool supports gradients.

- When saving icons, those at 256x256 pixels size are now png-encoded for
  smaller file size.

- Added DWIM-support for shrink_selection, shrinks to the other half 
  like auto-crop. [FIXME: VERIFY]

- Added DWIM-support for raster rotation, to use the most common edge color
  for the hole left by the rotation.

- Added an overlay showing where a control point in a path ended up when
  adjusting it and using snapping.

- These dialogs are no longer modal, and allow interaction with e.g.
  colors while they're open:
  - resize dialog
  - rotate dialog
  - threshold dialog

- [SVG] New fillstyle for objects: 'none'. Mostly to appease svg. Only
  settable via scripting.

- [SVG] Reworked svg parser
 - Supports .svgz (zipped svg)
 - Geometric paths more permissive (for example allows a single point)
 - Support for linked color stops
 - Support for svg 'currentColor'
 - Added limited svg 'viewBox' support
 - Added (deprecated!) CSS system colors to increase svg test suite
   coverage. Hard coded them to the Hot dog stand theme[1].
 - Added unit-to-pixel conversions for absolute units when loading svg [which?]
 - Path: Added 's' and 'S' operators.
 - Path: Added 't' and 'T' operators.
 - Added svg 'switch'-element support.
- [Python] Added: `obj.get_text_evaluated`.

- [Python] Added: `obj.get_text_raw`

- [Python] New object method: `obj.set_name(s)`, sets the name used for
  referring to objects from expressions in text objects.

- [Python] New function, `perimeter` for getting the perimeter length of objects.

- [Python] New method: `Canvas.get_paint(x,y)`, returns the Paint at x,y, either
  the background color or an object fill.

- [Python] Added crop-method to Python raster and text objects.

- [Python] Added new function `encode_bitmap_png`

- [Python] New method `Bitmap.fill`

- [Python] New method `Bitmap.set_threshold`

- [Python] Added `list_fonts` function

- [Python] Added comparison operations for Objects, based on their
  identifiers.

- [Python] Added property Canvas.command_name to allow
  specifying the undo/redo name for an upcoming command, to provide
  meaningful alternatives to "Undo Python Commands (3)"

- [Python] Added functions for shrinking the image by erasing rows or
  columns:
  - `faint.util.erase_columns`
  - `faint.util.erase_rows`
  - `faint.util.erase_selection`

    Erase selection is bound to Ctrl+Delete

- [Python] Added `crop` method for Raster objects, which auto-crops them.

- [Python] Added `crop` method for Text objects, which auto-sizes them.

- [Python] Added `dialog_save_copy(image)` function, for showing the save-as
  dialog and saving an image without modifying the path the image uses
  in Faint or clearing its modified status. Useful e.g. for saving a
  raster copy of a vector image.

- [Python] Added object getter functions to `Frame`:  
  `f.get_objects()`  
  `f.get_selected()`

- [Python] Added `pinch_whirl` to `Canvas`.

- [Help] All Python functions for showing dialogs are listed in the help.

- [Help] Documented how each file format handles transparency.

- [Help] Added a license information page.

- [Help] Added Python names of settings to the settings overview.
  
### Removed
- Removed palette text file, using instead a built-in hard-coded palette.
  The palette can be still be modified using the Python `palette`-object.

- [Python] Removed the non-gaussian blur function

- [Python] Removed `obj.get_text` (in favor of `obj.get_text_evaluated`,
  `obj.get_text_raw`.

- [Python] Removed all `set_` and `get_` methods for settings from objects
  in favor of properties, e.g. `rect.bg = (255,0,255)` instead of
  `rect.set_bg(255,0,255)`

- [Python] Removed  
  `FrameProps.get_background_png_string`  
  `FameProps.set_background_color`  
  `FrameProps.set_background_png_string`  
  `FrameProps.set_bitmap`.  

    For `get_background_png_string`, use instead
    `FrameProps.get_bitmap_stamped`, which returns the background
    with selection stamped and `encode_bitmap_png` to encode as png.

    For `set_...` use instead `FrameProps.set_background` which takes
    a bitmap or color.
    The bitmap can be created from PNG data with `ifaint.bitmap_from_png`.

- [Python] Removed Bitmap method `flood_fill`, which did not check
  boundaries, in favor of `fill(...)`, which does.

- [Python] Renamed `Canvas.get_pixel` to `Canvas.get_paint`, to clarify
  that it may return a gradient or pattern.

### Changed
- Merged raster and object selection into the same tool.

- Images are no longer required to have a raster a background, a color
  can be used instead. This reduces memory use when loading large
  vector images. A background is created silently when needed instead.

- Increased max font size from 255 to 999.

- Allow loading gifs with errors in blocks if at least one frame was
  loaded OK. Warnings are shown for this instead of aborting load.

- Changed the appearance of the tool-bar to make the selected tool more
  apparent.

- Faint no longer retries to "connect" to another Faint instance after
  one attempt has failed, and instead starts a new instance directly.

- [SVG] When color parsing fails, a warning is set and the colors
  defaults to black instead of failing the load.
  (Work around for svg-test "suite coords-units-01-b.svg").

- [Python] `Canvas.get_paint(x,y)` (formerly `get_pixel`) has been modified to 
  always return a color from the background (never a gradient or pattern).

- [Python] `Canvas.Text` allows passing a point (two coordinates) instead of
  a rectangle (four coordinates), and then creates an auto-sized text object.

- [Python] Requests for Invalid object identifiers from a `FrameProps`
  results in `IndexError` instead of `ValueError`.

- [Python] Functions throw `MemoryError` in some cases when allocations
  fail. [FIXME: DOCUMENT].

- [Python] `Canvas.set_size` accepts patterns as well.

- [Python] `Canvas.set_size` size argument must now be parenthesized:
  `images[0].set_size((640,480),get_fg())`.

- [Python] function `insert_bitmap` now stamps any current floating before
  inserting the bitmap as the new selection.

- [Python] The `auto_crop` methods of Bitmap, Frame and Canvas return `True` if
  anything was copped, `False` otherwise.

- [Python] Changed exception type when clipboard open fails from `ValueError` 
  to `OSError`.

- [Python] Setters for integers and floats state the passed in value,
  not only the ranges when outside:  
    `>>> set_linewidth(9000)`    
    `ValueError: Argument for set_linewidth 9000.0 outside range [0, 255].`.

- [Python] `TypeError` instead of `ValueError` thrown for insufficient arguments
  to functions.

- [Python] Renamed object method `o.as_object_path` to `o.become_path`.

### Fixed
- Improved text placement to avoid clipping umlauts.
 
- Fixed floating raster selection appearing on top of objects when
  flattening an image.

- Fixed crash when pressing backspace with no selection active.

- Fixed flickering of color in HSL panel on Windows.

- Fixed loss of alpha when rotating via the rotate dialog.

- Fixed loss of alpha when sharpening/blurring.

- Fixed loss of alpha when adjusting brightness/contrast.

- Fixed crash bug on negative drop index of frames in the frame control.

- Made the raster selection tool eat settings only if a selection exists.  
    **Explanation**: When the raster selection tool is active and there is a raster
    selection, background color and transparency changes will affect that
    selection only. If the raster selection tool is active, but no raster
    selection exists in the active image, the global tool settings will be
    modified instead.

- Improved performance of Boundary fill.

- Improved performance of Gaussian blur.

- Improved caret placement when clicking in a text object. The caret
  is now placed to the left of the character if its left half is
  clicked, otherwise to the right.

- Improved brush-pixel centering for brushes with even size.

- Improved hit test for Text objects.

- Fixed flickering of the color indication in the color panel.

- Fixed error in undo for 90-degree rotations. [Fixme: check if in 0.23]

- Wacom tablet: Failures during initialization of a Wacom tablet yield
  error message instead of crash on assertion.

- Wacom Tablet: Tablet is no longer initialized if starting with
  --silent.

- Paths created from Ellipses (using Objects->Become Path) now
  retain angle-spans (arcs).

- Custom file formats that create no frame yet sets no error,
  are now treated as a load error instead of crashing on assertion.

- Made Horizontal/Vertical constraining of raster selection movement
  more sticky, as per the comment:
  "Lock the constraining to a direction if the distance is somewhat
  large, then prefer this direction when at lesser distances, so that
  constraining can be used also close to the original position after
  establishing a likely intended-constrain direction.".

- Improved initial brush constraining to avoid "bumps" before
  the intended direction is determined..

- When starting with `--silent`, some error message boxes are suppressed

- Made the dashed border around colors in the selected-color control more
  distinct. The dashed border indicates that the color refers to a selected
  object or the raster selection.

- Made the border indicating which color in the palette is being edited by
  the color dialog thicker.

- More intuitive drop location of frames for drag and drop in the frame control.

- Invert and Desaturate now targets the raster selection if there is one.

- Fixed data offset in saved 8-bit-bitmaps.

- Fixed errors in 1bpp cursor loading.

- [SVG] Set the background color when saving an SVG with uniform background
  color.

- [SVG] Removed extension `faint:halign` in favor of standard SVG text-anchor.

- [SVG] Fixed centering on load.

- [SVG] Fixed saving of no-fill/no-stroke objects.

- [SVG] Fixed SVG arrowhead save/load.

- [SVG] Show warning for failed items in <def>.

- [SVG] Default to unbounded text when loading.

- [SVG] Image length expressed as percentage. [Fixme: What?]

- [SVG] Allow unary negative coordinates without leading space in
  coordinate lists. [r4097]

- [SVG] Render up to the last valid point for polygons and polylines
  with an odd number of coordinates, according to SVG 1.1 Appendix F2.
  [r4097]

- [PDF] Fixed division error in pdf writing.

- [PDF] Added escaping of \ in text.

- [Python] Updated to Python 3.4.

- [Python] Fixed incorrect property names in documentation.

- [Python] Interpreter now allows unicode in commands, e.g.
  `Text((0,0,100,100),"åäö")`.

- [Python] When parsing of a property fails on type error, the error states what
  the passed in object was:
  `TypeError: an integer is required (got type str)`

- [Python] Permission denied-error text is now shown also in non-ascii
  locales when permission denied occured during save from interpreter
  even in non-ascii locales e.g. 
  `"Error: can't open file 'c:\temp.png' (error 5: Åtkomst nekad.)"`

- [Python] Fixed invalid implementation of comparison for Faint classes
  which eventually depleted the reference counts of `Py_True` and `Py_False`

- [Python] Made Popen call used by dot-format[2] (in `extra.py`)
  Linux-compatible.

- [Python] Added methods `next_up`, `next_down`, `next_left`, `next_right`,
  `next_ahead`, `next_behind` for selecting a new object relative to a
  selected object.

- [Python] Added properties for all settings directly on the objects.    
  `ellipse.linewidth = 23`  
  `ellipse.linestyle = 'ld'`  
  `text.font = 'comic sans ms'`

- [Python] Removed visible property for `Grid`. Using `enabled` for
  both snap and visibility.

- [Python] Added many more methods to Bitmap.
  [Fixme: List them, e.g. subbitmap]

- [Python] Added `blit`-function for blitting bitmaps. Should be used
  instead of `bitmap.blit` method:
  `> blit(src_bmp, top_left, dst_bmp)`

- [Python] Added module `extra.tesseract` for running tesseract-ocr[3]
  to digitize text in an image.

- [Python] `skew` and `angle` for `Tri` are now properties instead of
  functions.

- [Python] Comparison of `Canvas`, `LinearGradient`, `RadialGradient`
  and `Pattern` with unsupported types now yield error instead of "greater than".

- [Python] `image_props.add_frame` rejects negative size.

- [Python] Replaced several object methods with properties:  
  `o.get_name`, `o.set_name` to `o.name`  
  `o.get_angle`, `o.set_angle` to `o.angle`   
  `o.tri(), o.set_tri(...)` to `o.tri`  
  `o.pos()`, `o.moveto()` to `o.pos`  
  `o.get_type()` to `o.type`  

- [Help] Fixed incorrect range for alpha in cursor format documentation.

### References
[1] http://blog.codinghorror.com/a-tribute-to-the-windows-31-hot-dog-stand-color-scheme/

[2] http://www.graphviz.org/

[3] https://code.google.com/p/tesseract-ocr/

## [0.23] - 2014-01-13 `r3623`

The last release available via google-code as they no longer support downloads.

### Added
- The grid can be offset with grid.anchor, and is automatically offset when
  the image is resized to the left and up (so that the grid remains over
  the same objects).

- Added module faint.extra.dot for loading GraphViz dot. Requires that
  dot is available on the path.

- Paths initialized from objects with "Become Path" start with point
  editing enabled, so that the change is more obvious. Editing points
  is a likely reason to turn an object into a path.

- Show the Save As dialog when using "Save" on a modified file with an
  extension for which there's no save function available, instead of
  showing the error message "No format available [...]"

- Added weak handling of text anchor in SVG, for better handling of
  the SVG produced by dot.

- Properties are documented in the help for the Python classes Canvas,
  FaintApp, Grid and Settings.

- FaintApp.open docstring improved.

### Fixed
- Fixed crash when adjusting last control point of Path object
  and constraining with shift

- Remove <faint-root>/py when uninstalling

- Path created from polygon is closed

- [Python] Fixed broken `anchor_flagged` function

- [Python] Fixed broken `reverse_frames` function

## [0.22] [2014-01-12] `r3607`

### Added
- Preview checkbox in all dialogs can be toggled with 'P'-key
- Objects can be converted to paths using Objects->Become path
- [Python] Function `to_svg_path now` supports all object types
- [Help] More Python interface functions are documented in the help (the
  script that harvests docstrings was improved)
- [Help] Improved documentation for Python Tri-object

### Changed
- [Python] Renamed the package containing the svg and pdf formats
  to "faint" from "py".

### Fixed
- [Python] Fixed broken Python function `unbindk`.
- [Python] Fixed broken Python function `toggle_flag_pixel`.
- [Python] Added error check ensuring that a bound function is callable.
- [Python] Added method `app.open` as an alternative to Canvas constructor.
- Pattern text controls are reset when pasting pattern.
- Polygon and Path support aligned resize.
- Removed an incorrect encode causing svg load to fail on some text
  objects.

## [0.21] 2014-01-08  `r3569`
Fixes some things that broke in Faint 0.20.

### Added
- Made the Python interpreter appear on application start if there is
  an error in the user's configuration script or a script specified
  with --run, so that the error output is shown.

- [Python] Line drawing function for Python Bitmap: `line(x0,y0,x1,y1)`.

- [Python] Improved documentation string for Python Canvas and Frame
  `aa_line`-function.

### Fixed
- Fixed running Python-scripts with --run on the command line
- Fixed Python 33 byte-prefix appearing before the svn revision in the about
  dialog.
- Fixed assertion failure on errors in users configuration script.

## [0.20] 2014-01-06 
`r3559`
For Faint 0.20, there were some large changes. The Visual Studio
version was changed from 2012 to 2013, Python from Python 2.7 to
Python 3.3 and most remaining 7/8-bit string handling was reworked to
use Unicode.

### Added
- Added sharpness-dialog
- Added unsharp mask filter.
- Added gaussian blur filter.
- Added color balance filter and dialog
- [Help] Added help text about the path tool.

### Changed
- Changed Python version to Python 3.
- Replaced the spline-tool with the path-tool in the tool bar.

### Removed
- Removed broken Python function: `reload_ini`.

- Removed support for DWIM-delete by deleting the same region twice.
  Alt+Y after delete still triggers DWIM.

### Fixed
- Fixed error in boundary fill (color and pattern) which caused
  vertical 1-pixel lines to not get filled.

- Fixed bug which caused point-snapping to prefer snap points in
  objects with lower Z (closer to back) to points in objects with
  higher Z, even if closer (x,y)

- Selected color control indicates with a dashed border around a color
  if it targets a selection color rather than the common color
  setting.

- Raster selection rectangle is constrained to a square when shift is
  held when creating

- Python-implemented file formats (svg, pdf) also support
  unicode file names with > ascii characters.

- Fixed crash when capturing mouse twice in sliders (e.g. on left
  down by double-tap touch pad and then left button).

- [Python] Fixed error in Python Canvas-`__init__`-function for multiple
  filenames.

- [Python] Fixed crash when undoing all commands during creation of a "command
  bundle" via Python in a loop, so that there were no command left
  Example:  
  `for i in range(10):`  
  `..line((0,i,10,i),get_settings())`  
  `..undo()`

- [Python] Fixed crash when undoing more commands than available during creation
  of a "comand bundle" via Python. Example:  
  `for i in range(10):`  
  `..line((0,i,10,i),get_settings())`  
  `..undo()`  
  `..undo() # Oops`

- [Python] function `browse_to_active_file` (key:E) supports unicode file
  names on Windows.

- [Python] Added function for selecting a specific frame:
  `canvas.select_frame(index)`

- [Python] Fixed crash when setting negative grid spacing.

- [Python] Fixed an argument parse error for Bitmap arguments.

- [Python] Added Python function to app-object for checking if alpha blending
  towards a checkered pattern is used for indicating transparency:
  app.get_checkered_transparency_indicator()

- [Python] Added Sepia filter for no good reason at all. Available only from
  interpreter: `image.Sepia(i)`

- Menu items were not refreshed when deselecting with the
  object selection tool. Hence deselections which should have enabled
  moving objects forward or backward did not.

- Added setting for closed/open paths.

- Path tool reacts immediately to point-type changes instead of after
  completing the current point.

- No resize command is created by the Resize canvas tool if the size
  is unchanged. This avoids flagging the image as dirty on a no-change
  resize.

- Added cursors for when the level tool is defining a horizon and when
  scaling is used.

- Fixed Pinch-whirl not supporting negative angles.

- Object resize tool will not set invalid Tri:s to objects.

- Grid GUI-control is now closed when changing tab from a Canvas with
  a grid to one without.

- Corrected the anchor for the brush overlay when inactive, so that
  the overlay graphic shows the pattern area that would be drawn on a
  click.

- Added supports for moving points in Path-objects

- Added more status bar feedback for the polygon tool: angle, and line length.

- [Help] Improved documentation for some tools: path, polygon, line, text.
