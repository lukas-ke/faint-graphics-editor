#!/usr/bin/env python3
import os
import shutil
import sys
from faint_pyd_paths import SRC_FILE, DST_DIR, DST_FILE

if sys.platform.startswith('linux'):
    print("Not implemented.")
    exit(1)

if not os.path.exists(DST_DIR):
    print("Folder %s is missing!" % DST_DIR)
    exit(1)

if os.path.exists(DST_FILE):
    shutil.remove(DST_FILE)
else:
    print("File not found: %s" % DST_FILE)
