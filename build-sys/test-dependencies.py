#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os
import sys
import build_sys.dependencies as dep

def fwd(p):
    return p.replace("\\", "/")

ROOT = fwd(os.path.abspath(".")) + "/"
FOLDERS = ["test-sources"]

jp = os.path.join

def test_sources(file_name):
    return fwd(jp(ROOT, "test-sources", file_name))

def test_enumerate_all_sources(verbose):
    if verbose:
        print("\ntest_enumerate_all_sources")
    src = sorted(dep.enumerate_all_sources(ROOT, FOLDERS))

    if verbose:
        for item in src:
            print(" " + item)

    assert src == [
        test_sources("header1.hh"),
        test_sources("header2.hh"),
        test_sources("source1.cpp"),
        test_sources("source2.cpp")]


def test_get_flat_header_dependencies(verbose):
    if verbose:
        print("\ntest_get_flat_header_dependencies")
    items = dep.get_flat_header_dependencies(ROOT, FOLDERS)
    assert len(items) == 2 # Two headers
    if verbose:
        for k in sorted(items.keys()):
            print(k)
            for item in sorted(items[k]):
                print(" -> ", item)
            print()

    assert sorted(list(items[test_sources("header1.hh")])) == [
        test_sources("source1.cpp")]

    # Header 2 is included directly by source2.cpp,
    # and indirectly via source1.cpp
    assert sorted(list(items[test_sources("header2.hh")])) == [
        test_sources("source1.cpp"),
        test_sources("source2.cpp")]

def test_find_header_dependencies_all(verbose):
    if verbose:
        print("\ntest_find_header_dependencies_all")

    items = dep.find_header_dependencies_all(ROOT, FOLDERS)
    if verbose:
        for k in sorted(items.keys()):
            print("\n" + k)
            for item in sorted(list(items[k])):
                print("->" + item)

    assert len(items) == 2 # Two headers
    assert sorted(list(items[test_sources("header1.hh")])) == [
        test_sources("source1.cpp")]
    assert sorted(list(items[test_sources("header2.hh")])) == [
        test_sources("header1.hh"),
        test_sources("source2.cpp")]

if __name__  == '__main__':
    verbose = "-v" in sys.argv

    test_enumerate_all_sources(verbose)
    test_get_flat_header_dependencies(verbose)
    test_find_header_dependencies_all(verbose)
    print("OK")
