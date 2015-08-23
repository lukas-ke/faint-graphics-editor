def load_tests(loader, standard_tests, pattern):
    return loader.discover(".")
