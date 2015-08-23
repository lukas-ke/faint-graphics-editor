import os

def load_tests(loader, standard_tests, pattern):
    return loader.discover(".")

TEST_OUT_PATH = None

def make_test_dir(test):
    if TEST_OUT_PATH is None:

        raise ValueError("TEST_OUT not set.")
    if not os.path.exists(TEST_OUT_PATH):
        os.mkdir(TEST_OUT_PATH)

    path = os.path.join(TEST_OUT_PATH, "py-" + test._testMethodName)
    if not os.path.exists(path):
        os.mkdir(path)

    return path
