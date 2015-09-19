#!/usr/bin/env python3
# -*- coding: utf-8 -*-

"""Test which runs Faint on svg:s from the svg-test suite and saves
them to the output formats given on the command line (available
arguments are defined by TARGET_INFO).

"""

import optparse
import configparser
import os
import subprocess
import shutil
import svg_suite.config as config
from svg_suite.html_index import write_html_index, target_name
import sys
import traceback

# Use the .pyd in ext/out
FAINT_ROOT = os.path.abspath(os.path.join(os.getcwd(), ".."))
EXT_OUT = os.path.join(FAINT_ROOT, "ext/out")
PY = os.path.join(FAINT_ROOT, "py")
sys.path.insert(1, EXT_OUT)
sys.path.insert(1, PY)

import faint.svg.parse_svg as svg
import faint
from faint import Bitmap

# Console width, All output is adjusted (ljust) to this width to
# clear old output after stepping back with '\b', so the
# progress feedback can be written on the same line.
COLUMNS = 60

# The script used for saving the loaded svgs.
SCRIPT = 'faint_scripts/test-save-suite.py'


def get_target_dir(target_type):
    return os.path.join(os.getcwd(), 'out', 'suite', target_type)


TARGET_INFO = {
    'pdf': {'cleaned_exts': ('.pdf',),
            'target_dir': get_target_dir('pdf')},

    'svg': {'cleaned_exts': ('.svg', '.svgz'),
            'target_dir': get_target_dir('svg')},

    'png': {'cleaned_exts': ('.png', '.html',),
            'target_dir': get_target_dir('png')},
}


def get_heading(target_type, test_num, total_tests):
    return ("=== %s (%d/%d) ===" %
            (target_type, test_num + 1, total_tests)).ljust(COLUMNS) + "\n"


def get_info_str(target_type, file_num, svg_files):
    num_files = len(svg_files)
    file_name = os.path.split(svg_files[file_num])[1]
    return ("%s: %d / %d %s" %
            (target_type,
             file_num + 1, num_files,
             file_name.strip())).ljust(COLUMNS)

def get_out_error(source_file, target_file, error_num):
    return (' ' * COLUMNS + '\n' +
            ('Error #%d: output file' % error_num).ljust(COLUMNS) + '\n' +
            (' ' + target_file).ljust(COLUMNS) + '\n' +
            ' not created from'.ljust(COLUMNS) + '\n' +
            (' ' + source_file).ljust(COLUMNS) + '\n')


def get_blank_line():
    return ' ' * COLUMNS + '\n'

old_info_str_len = 0
error_num = 0
def test_file(num, f, target_dir, target_type, silent, svg_file_paths):
    global old_info_str_len
    global error_num
    l = faint.PimageList()
    svg.parse_doc(f, l)
    img = l.frames[0]
    img.flatten()
    target_file = os.path.join(target_dir, target_name(f, target_type))
    faint.write_png(img.background, target_file, faint.png.RGB)
    if not os.path.exists(target_file):
        if old_info_str_len != 0 and not silent:
            sys.stdout.write("\b" * old_info_str_len)

        if error_num > 0 and not silent:
            sys.stdout.write(get_blank_line())
        error_num += 1
        sys.stdout.write(get_out_error(f, target_file, error_num))

        old_info_str_len = 0

    if not silent:
        info_str = get_info_str(target_type,
                            num, svg_file_paths)

        sys.stdout.write("\b" * old_info_str_len + info_str)
        old_info_str_len = len(info_str)
        sys.stdout.flush()


def run_test(svg_test_suite_root,
             target_type,
             svg_file_paths,
             test_num,
             total_tests, silent):
    total_fails = []

    target_info = TARGET_INFO[target_type]
    target_dir = target_info['target_dir']

    if total_tests > 1 and not silent:
        sys.stdout.write(get_heading(target_type, test_num, total_tests))

    if target_type == 'png':
        write_html_index(target_dir, 'png', svg_file_paths, svg_test_suite_root)

    for num, f in enumerate(svg_file_paths):
        try:
            test_file(num, f, target_dir, target_type, silent, svg_file_paths)
        except Exception as e:
            total_fails.append((f, e))

    return total_fails


def setup_test(target_type):
    """Recreate the output folder for the specified type."""
    target_info = TARGET_INFO[target_type]
    cleaned_exts = target_info['cleaned_exts']
    target_dir = get_target_dir(target_type)
    config.recreate_out_dir(target_dir, cleaned_exts)


def parse_options():
    optParser = optparse.OptionParser()
    return optParser.parse_args()


def test_svg_suite(cfg, args, silent=False):
    assert len(args) != 0

    if 'all' in args:
        assert len(args) == 1
        args = list(TARGET_INFO.keys())

    cfg = config.read_config()
    svg_test_suite_root = cfg.get('test','svg_test_suite_root')
    svg_files = config.list_svg_files(svg_test_suite_root)

    for num, target_type in enumerate(sorted(args)):
        setup_test(target_type)
        total_fails = run_test(svg_test_suite_root, target_type, svg_files, num,
                               len(args), silent)
    print()
    print("Total failures: %d/%d" % (len(total_fails), len(svg_files)))

    if len(total_fails) != 0:
        with open("tracebacks.log", 'w') as f:
            for failure in total_fails:
                f.write("%s, %s" % (failure[0], str(failure[1])))
                f.write("".join(traceback.format_exception(failure[1].__class__, failure[1], failure[1].__traceback__)))


if __name__ == '__main__':
    if config.maybe_create_config():
        exit(1)

    opts, args = parse_options()

    if len(args) == 0:
        targets = "all " + " ".join(TARGET_INFO.keys())
        print("No arguments specified. Expected: %s" % targets)
        exit(1)

    if 'all' in args:
        if len(args) != 1:
            print('Error: No additional arguments allowed with "all".')
            exit(1)

    cfg = config.read_config()
    test_svg_suite(cfg, args)
