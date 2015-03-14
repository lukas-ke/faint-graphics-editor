#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Test which creates a PDF, saves it and opens it.

"""

import subprocess
import sys

import src.config as config

def test_save_pdf(cfg, silent=True):
    faint_exe = cfg.get('test', 'faint_exe')
    cmd = [faint_exe, '--run', 'faint_scripts/test-save-pdf.py']
    if silent:
        cmd.append('--silent')
    subprocess.call(cmd)


if __name__ == '__main__':
    if config.maybe_create_config():
        exit(1)
    silent = not 'stay' in sys.argv
    cfg = config.read_config()
    test_save_pdf(cfg, silent)
