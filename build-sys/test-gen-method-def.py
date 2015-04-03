#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import build_sys.gen_method_def as gen

if len(sys.argv) == 2:
    gen.check_file(sys.argv[1])

elif len(sys.argv) == 5:
    src, dst, dst_doc, name = sys.argv[1:]
    gen.generate(src, dst, dst_doc, name)
