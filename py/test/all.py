#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import sys
import src.config as config
from test_svg_suite import test_svg_suite
from test_py_api import test_py_api
from test_color_background import test_color_background
from test_props import test_props
from test_svg_util import test_svg_util
from test_save_svg import test_save_svg
from test_save_pdf import test_save_pdf

def run_test(func, *args, **kwArgs):
    print(func.__name__)
    func(*args, **kwArgs)


def run_if(c, func, *args, **kwArgs):
    if (c):
        run_test(func, *args, **kwArgs)
    else:
        print("Skipping: " + func.__name__)


if __name__ == '__main__':
    if config.maybe_create_config():
        exit(1)



    fast = 'fast' in sys.argv
    suite_tests = ['svg'] if 'svgonly' in sys.argv else ["all"]

    cfg = config.read_config()
    run_test(test_svg_util)
    run_test(test_py_api, cfg, silent=True)
    run_test(test_color_background, cfg, silent=True)
    run_test(test_props, cfg, silent=True)
    run_test(test_save_svg, cfg, silent=True)
    run_test(test_save_pdf, cfg, silent=True)
    run_if(not fast, test_svg_suite, cfg, suite_tests, silent=True)

