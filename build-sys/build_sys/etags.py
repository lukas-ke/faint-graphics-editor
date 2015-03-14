# Copyright 2013 Lukas Kemmer
#
# Licensed under the Apache License, Version 2.0 (the "License"); you
# may not use this file except in compliance with the License. You
# may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
# implied. See the License for the specific language governing
# permissions and limitations under the License.

import ConfigParser
import os
import subprocess
import faint_info

if not os.path.exists( "build.cfg" ):
    print "Error: build.cfg missing. Run build.py first."
    exit(1)

# Get the path to the etags application
config = ConfigParser.RawConfigParser()
config.read('build.cfg')
etags_folder = config.get("other", "etags_folder")
etags_exe = os.path.join(etags_folder, "etags.exe")

# List all Faint source files
sourceFiles = ['"%s"' % file for file in faint_info.enumerate_all_sources(faint_info.faintRoot)]

etags = subprocess.Popen('"%s" %s' % (etags_exe, " ".join(sourceFiles)))
if etags.wait() != 0:
    print "Error generating TAGS file."
    exit(1)
