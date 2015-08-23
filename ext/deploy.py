#!/usr/bin/env python3
import os
import sys
import shutil
from faint_pyd_paths import SRC_FILE, DST_DIR, DST_FILE

if sys.platform.startswith('linux'):
    print("Not implemented.")
    exit(1)

if not os.path.exists(SRC_FILE):
    print("%s not found!" % SRC_FILE)
    exit(1)

if not os.path.exists(DST_DIR):
    print("Folder %s missing!" % DST_DIR)
    exit(1)

print("Copying %s to %s" % (SRC_FILE, DST_DIR))
shutil.copy(SRC_FILE, DST_FILE)
