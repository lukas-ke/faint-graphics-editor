#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import build_sys.gen_help as gen

force = "--force" in sys.argv
gen.run(force)
