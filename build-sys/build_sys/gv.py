#!/usr/bin/env python3
# -*- coding: utf-8 -*-

# Copyright 2014 Lukas Kemmer
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

"""Generating dependency graph sources for GraphViz dot"""

import os

def _gv_name(file_path):
    return os.path.basename(file_path).replace("-","").replace(".","")


def _recurse_gv(includeDict, parent, dependencySet, nameSet, names):
    if parent in includeDict:
        for child in includeDict[parent]:
            parent_color = "blue" if parent in names else "wheat"
            child_color = "blue" if child in names else "wheat"

            dependencySet.add("%s -> %s;" % (_gv_name(parent), _gv_name(child)))
            nameSet.add('%s [label="%s" fillcolor=%s style=filled]' % (_gv_name(child), os.path.basename(child), child_color))
            nameSet.add('%s [label="%s" fillcolor=%s style=filled]' % (_gv_name(parent), os.path.basename(parent), parent_color))
            _recurse_gv(includeDict, child, dependencySet, nameSet, names)


def follow_includes_graph(includeDict, names=None):
    """Prints a directed graph which follows all includes originating from
    the files listed in names"""
    dependencySet = set()
    nameSet = set()
    print("digraph G{")
    if names is None:
        names = includeDict.keys()
    for filename in names:
        _recurse_gv(includeDict, filename, dependencySet, nameSet, names)
    for name in nameSet:
        print(name)
    for item in dependencySet:
        print(item)

    print("}")


def recurse_who_includes(includeDict, name, nameSet, dependencySet):
    name_font = "Helvetica-bold" if name.endswith(".hh") else "Helvetica"
    nameSet.add('%s[label="%s" color="none", fontname="%s", fontsize="20"]' % (_gv_name(name), os.path.basename(name), name_font))

    for child in includeDict.get(name, []):
        name_font = "Helvetica-bold" if child.endswith(".hh") else "Helvetica"

        recurse_who_includes(includeDict, child, nameSet, dependencySet)
        nameSet.add('%s[label="%s" color="none" fontsize="20", fontname="%s"]' % (_gv_name(child), os.path.basename(child), name_font))
        dependencySet.add('%s->%s[weight=0.1,arrowsize=0.4,penwidth=1.2];' % (_gv_name(child), _gv_name(name)))

def who_includes(includeDict, name):
    """Finds all files who includes name, and recurses outwards."""
    nameSet = set()
    dependencySet = set()
    recurse_who_includes(includeDict, name, nameSet, dependencySet)
    result = "digraph G{\n"
    result += 'graph[fontname="Helvetica",fontsize=13,ranksep=3.000,overlap=false];\n'
    result += 'root=%s;\n' % _gv_name(name)
    result += 'edge[color="#4D6DF3"];\n'
    for entry in nameSet:
        result += (entry + "\n")
    for entry in dependencySet:
        result += (entry + "\n")
    result += "}\n"
    return result

def test_follow_includes_graph():
    import sys
    from faint_info import _mapped_includes
    root = sys.argv[1].replace("\\","/")
    if len(sys.argv) > 2:
        names = sys.argv[2:]
    else:
        names = None
    # Find all include dependencies
    files_to_includers = _mapped_includes(root)

    # Print a graph
    follow_includes_graph(files_to_includers, names)


def test_who_includes_graph():
    import sys
    from faint_info import find_header_dependencies_all
    if len(sys.argv)!= 3:
        return

    root = sys.argv[1].replace("\\","/")
    name = sys.argv[2].replace("\\","/")

    include_dict = find_header_dependencies_all(root)
    who_includes(include_dict,name)

if __name__ == '__main__':
    test_who_includes_graph()
