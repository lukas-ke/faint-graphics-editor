#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import re
import os
import sys
from faint_scripts.log import Logger
sys.path.append(os.path.join(os.getcwd(), "mock"))
sys.path.append(os.path.join(os.getcwd(), ".."))

import faint.svg.svg_re as svg_re
import faint.svg.util as util

l = Logger("test_svg_util.py")

def _regex_test():
    l.Assert(re.match(svg_re.number_attr, "10.1").group(0) == '10.1')
    l.Assert(re.match(svg_re.number_attr, "10").group(0) == '10')
    l.Assert(re.match(svg_re.number_attr, "10.10").group(0) == '10.10')

    l.Assert(re.match(svg_re.length_attr, "-640.0E10").groups() == ('-640.0E10',''))
    l.Assert(re.match(svg_re.length_attr, "-640pt").groups() == ('-640','pt'))
    l.Assert(re.match(svg_re.length_attr, "640pt").groups() == ('640','pt'))

def test_svg_util():
    _regex_test()


if __name__ == '__main__':
    test_svg_util()
