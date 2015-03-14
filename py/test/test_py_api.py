#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Test which exercises the Faint Python api.

"""

import subprocess
import src.config as config
import sys

def test_py_api(cfg, silent=True):
    faint_exe = cfg.get('test', 'faint_exe')
    cmd = [faint_exe, '--run', 'faint_scripts/run-test-py-api.py']
    if silent:
        cmd.append('--silent')
    subprocess.call(cmd)


if __name__ == '__main__':
    if config.maybe_create_config():
        exit(1)

    silent = not 'stay' in sys.argv
    cfg = config.read_config()
    test_py_api(cfg, silent)
