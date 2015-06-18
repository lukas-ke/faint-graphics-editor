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
import faint.svg.parse.parse_state as parse_state
from faint.svg.parse.parse_util import Matrix

l = Logger("test_svg_util.py")

def _regex_test():
    l.Assert(re.match(svg_re.number_attr, "10.1").group(0) == '10.1')
    l.Assert(re.match(svg_re.number_attr, "10").group(0) == '10')
    l.Assert(re.match(svg_re.number_attr, "10.10").group(0) == '10.10')

    l.Assert(re.match(svg_re.length_attr, "-640.0E10").groups() == ('-640.0E10',''))
    l.Assert(re.match(svg_re.length_attr, "-640pt").groups() == ('-640','pt'))
    l.Assert(re.match(svg_re.length_attr, "640pt").groups() == ('640','pt'))


def _matrix_test():
    def matrix_near(m1, m2):
        d = 0.001
        def near(v1, v2):
            return abs(v1 - v2) < d

        return all([near(v1, v2) for v1, v2 in zip(m1.values(), m2.values())])

    scale, rotate, translate = parse_state.parse_transform_list(
        "scale(3 2) rotate(3) translate(4 112)")

    l.Assert(matrix_near(Matrix.scale(3,2), scale))
    l.Assert(matrix_near(Matrix.rotation(0.05235), rotate))
    l.Assert(matrix_near(Matrix.translation(4, 112), translate))


def test_svg_util():
    _regex_test()
    _matrix_test()

if __name__ == '__main__':
    test_svg_util()
