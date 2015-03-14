import sys
import build_sys.genhelp as gen

force = "--force" in sys.argv
gen.run(force)
