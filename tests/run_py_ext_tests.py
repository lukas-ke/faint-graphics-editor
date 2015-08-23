#!/usr/bin/env python3
import unittest

def setup_path():
    # Use the .pyd in ext/out
    import os, sys
    FAINT_ROOT = os.path.join(os.getcwd(), "..")
    EXT_OUT = os.path.join(FAINT_ROOT, "ext", "out")
    sys.path.append(os.path.abspath(EXT_OUT))

if __name__ == '__main__':
    setup_path()
    import py_ext_tests
    unittest.main(py_ext_tests)
