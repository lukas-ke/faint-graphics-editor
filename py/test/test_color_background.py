#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import configparser
import os
import subprocess
import shutil
import src.config as config
import sys

def test_color_background(cfg, silent):
    faint_exe = cfg.get('test', 'faint_exe')
    cmd = [faint_exe, '--run', 'faint_scripts/run-test-color-background.py']
    if silent:
        cmd.append('--silent')
    subprocess.call(cmd)

if __name__ == '__main__':
    if config.maybe_create_config():
        exit(1)

    silent = not 'stay' in sys.argv
    cfg = config.read_config()
    test_color_background(cfg, silent)
