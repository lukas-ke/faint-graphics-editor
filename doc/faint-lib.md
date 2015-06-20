Splitting Faint into libs
=========================
The Faint code should be made less tangled, so that parts are usable
and testable on their own by separation into libraries.

Unfortunately this is rather boring.

As an example of current limitations: to test the SVG-parsing
(implemented in Python), I currently need to run Faint with
`--silent`, passing it a script. It would be much nicer if I could do
`parse_svg` and get some `faint.Image` object in a regular Python script with the
normal Python interpreter.

The Faint scripting is getting increasingly capable, and having to run
the Faint gui application for non-interactive scripts is a limitation.

Having the useful C++-parts (without the Python-api) available
separately for other C++-applications would also be nice.

Thoughts
--------
The "contextual" application code needs to be separated from the
separately useful.
  
I could either make a large lib with "most" stuff, or several libs. If
I do several libs (e.g. geo, util, bitmap...), I might need to figure
out dependencies, or I could just require linking most of them, though
that sucks a little.

I should move all context-heavy code into app/. This will include some
of the C++-Python-api (currently in /Python).

- I want to avoid having to do e.g. #include "faint-lib/..." from the
Faint application code to get at the commons.

What goes in the C++-libs?
--------------------------
The separation is probably something like:

1. regular-good-old-code
2. gui
3. main (application etc.)
4. python depending on regular-good-old-code
5. python depending on gui and regular good-old-code

Faint = 1+2+3+4+5
Python-lib = 1+4

GUI is not simply *anything wxWidgets*, since I use e.g. `wxFileName`
behind the scenes in `FilePath`.

### Reasonably libbable
- All geometry classes
- `Bitmap`
- `CairoContext`
- Draw functions (`draw.cpp`)
- `Filter`
- File formats
- `FaintDC`
- All objects
- All geometry
- Select parts of `Util/`
...?

In general, `get_app_context` and `get_art` are large hindrances.
