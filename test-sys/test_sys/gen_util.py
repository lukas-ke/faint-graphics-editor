import os
from os.path import getmtime

def file_name_to_function_call(f):
    return f.replace('.cpp', '();').replace('-','_')

def file_name_to_function_pointer(f):
    return f.replace('.cpp', '').replace('-','_')

def file_name_to_declaration(f):
    return 'void ' + f.replace('.cpp', '();').replace('-','_')

def file_name_to_impl_line(f):
    return 'void ' + f.replace('.cpp', '()').replace('-','_')

def need_generate(root_dir, out_file, files):
    if not os.path.exists(out_file):
        return True

    files = [os.path.join(root_dir, f) for f in files]

    newest = list(sorted(files, key=lambda f: getmtime(f)))[-1]
    return getmtime(newest) > getmtime(out_file)

def check_file(root_dir, file_name):
    """Checks that the specified file contains a correctly named test
    function.

    """
    file_path = os.path.join(root_dir, file_name)
    with open(file_path, 'r', encoding="utf-8") as f:
        if f.read().find(file_name_to_impl_line(file_name)) == -1:
            print("Error: %s must have a function named\n %s" %
                  (file_name, file_name_to_impl_line(file_name)))
            exit(1)
