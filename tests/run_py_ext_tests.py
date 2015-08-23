#!/usr/bin/env python3
import os
import py_ext_tests
import sys
import unittest

def run_tests(test_args=None):
    if (test_args == None):
        test_args = ["run_py_ext_tests",]
    # Use the .pyd in ext/out
    FAINT_ROOT = os.path.abspath(os.path.join(os.getcwd(), ".."))
    EXT_OUT = os.path.join(FAINT_ROOT, "ext/out")
    py_ext_tests.TEST_OUT_PATH = os.path.join(FAINT_ROOT, "tests/out")
    sys.path.insert(1, os.path.abspath(EXT_OUT))

    # Run the tests

    tp = unittest.main(py_ext_tests, argv=test_args, exit=False)
    result = tp.result
    return len(result.errors) == 0

if __name__ == '__main__':
    exit(run_tests(sys.argv))
