#!/usr/bin/env python3
import unittest

def run_tests():
    # Use the .pyd in ext/out
    import os, sys
    FAINT_ROOT = os.path.join(os.getcwd(), "..")
    EXT_OUT = os.path.join(FAINT_ROOT, "ext", "out")
    sys.path.append(os.path.abspath(EXT_OUT))

    # Run the tests
    import py_ext_tests
    tp = unittest.main(py_ext_tests, exit=False)
    result = tp.result
    return len(result.errors) == 0

if __name__ == '__main__':
    exit(run_tests())
