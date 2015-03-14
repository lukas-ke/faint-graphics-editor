import configparser
import os
from . svg_test_files import TESTABLE_FILES

def _create_config():
    with open('config.ini', 'w') as f:
        f.write('[test]\n')
        f.write('svg_test_suite_root=\n')
        f.write('faint_exe=\n')

    print("Created config file config.ini.")
    print("Update this file and run the test again.")


def read_config():
    config = configparser.RawConfigParser()
    config.read('config.ini')
    return config

def maybe_create_config():
    if not os.path.exists('config.ini'):
        _create_config()
        return True
    return False

def matches_ext(filename, exts):
    for e in exts:
        if filename.endswith(e):
            return True
    return False

def recreate_out_dir(out_dir, cleaned_exts):
    if os.path.exists(out_dir):
        for filename in os.listdir(out_dir):
            if matches_ext(filename, cleaned_exts):
                os.remove(os.path.join(out_dir, filename))
        os.rmdir(out_dir)
    os.makedirs(out_dir)
    return out_dir

def list_svg_files(svg_test_suite_root):
    svg_file_root = os.path.join(svg_test_suite_root, 'svg')
    svg_files = os.listdir(svg_file_root)
    svg_files = [os.path.join(svg_file_root, f) for f in svg_files if
                 f in TESTABLE_FILES]
    return svg_files
