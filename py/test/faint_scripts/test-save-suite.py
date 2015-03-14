import os
from ifaint import cmd_arg

"""Script be passed to Faint with --run by test-svg-suite.py.
Requires that the type is passed with --arg as well, e.g.
> Faint --run test-save.py --arg png

"""

outdir = os.path.join(os.getcwd(), 'out', 'suite', cmd_arg)
active = get_active_image()
filename = active.get_filename()

if filename is not None:
    ext = "." + cmd_arg
    fn = os.path.basename(active.get_filename())
    fn = fn.replace('.svgz', ext)
    fn = fn.replace('.svg', ext)
    get_active_image().save_backup(os.path.join(outdir,fn))
