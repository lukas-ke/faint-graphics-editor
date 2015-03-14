#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Script passed as --run argument to Faint by test-py-api.py.

Imports the actual test implementation, to be able to inspect it for
outputting failed source lines.

"""
from inspect import getsourcelines as gsl
import inspect
import faint_scripts.test_py_api_impl as test_py_api_impl
from faint_scripts.log import Logger

log = Logger("run-test-py-api.py")

def fail_if(cond):
    if cond:
        with open("errors.log", 'a') as f:
            source_lines, start_line = gsl(test_py_api_impl.test)
            line_number = inspect.stack()[1][2]
            expression = source_lines[line_number - start_line].strip()
            log.error("fail if on line %d: %s" %
                      (line_number, expression))

test_py_api_impl.test(fail_if)
