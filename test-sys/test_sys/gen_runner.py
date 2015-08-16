import os
import sys
import test_sys.gen_util as util

class test_runner_info:
    test_type = "Test"
    extra_includes = ["test-sys/run-test.hh"]
    extra_globals = []
    main_function_name = "run_tests"
    test_function_name = "run_test"

    args = ['  const bool silent = find_silent_flag(argc, argv);',]

    def write_function_call(self, out, func, file_name, max_width):
        out.write('  test::run_test(%s, "%s", %d, numFailed, silent);\n'
                  % (func, file_name, max_width))


class bench_runner_info:
    test_type = "Bench"
    extra_includes = ["test-sys/run-bench.hh"]
    extra_globals = ["std::vector<Measure> BENCH_MEASURES"]
    main_function_name = "run_benchmarks"
    test_function_name = "run_bench"
    args = ['  const std::string benchmarkName = find_test_name(argc, argv);']

    def write_function_call(self, out, func, file_name, max_width):
        out.write('  if (benchmarkName.empty() || benchmarkName == "%s"){\n' % file_name)
        out.write('    run_bench(%s, "%s");\n'
                     % (func, file_name))
        out.write('  }\n')


class image_runner_info:
    test_type = "Image"
    extra_includes = ["test-sys/run-image.hh"]
    extra_globals = []
    main_function_name = "run_image_tests"
    test_function_name = "run_image"
    args = ['  const std::string testName = find_test_name(argc, argv);']

    def write_function_call(self, out, func, file_name, max_width):
        # Fixme
        out.write('  if (testName.empty() || testName == "%s"){\n' % file_name)
        out.write('    run_image(%s, "%s");\n'
                  % (func, file_name))
        out.write('  }\n')

def gen_runner(root_dir, out_file, info):
    all_files = [f for f in os.listdir(root_dir) if (
        f.endswith(".cpp") and
        not f.startswith('.'))]

    files = [f for f in all_files if f != 'stubs.cpp' and f != 'main.cpp']

    if not util.need_generate_single(out_file, __file__):
        if not util.need_generate(root_dir, out_file, files):
            print("* %s-runner up to date." % info.test_type)
            return

    print("* Generating %s-runner" % info.test_type)

    out_dir = os.path.dirname(out_file)
    if not os.path.exists(out_dir):
        os.mkdir(out_dir)

    max_width = max([len(f) for f in files])

    for file_name in files:
        util.check_file(root_dir, file_name)

    # If there is a user-supplied main.cpp, use "run_tests" as the
    # test function to allow calling from there.
    # Otherwise, define the main function.
    main_function_name = (info.main_function_name
                          if 'main.cpp' in all_files else
                          "main")

    with open(out_file, 'w') as out:
        out.write('// Generated by %s\n' % os.path.basename(__file__))
        out.write('#include <iostream>\n');
        out.write('#include <iomanip>\n');
        out.write('#include <sstream>\n');
        out.write('#include <vector>\n');
        out.write('#include "test-sys/test-sys.hh"\n')

        for include in info.extra_includes:
            out.write('#include "%s"\n' % include)

        if not sys.platform.startswith('linux'):
            # For disabling error dialogs on abort
            out.write('#include "windows.h"\n')
        out.write('\n');

        out.write('namespace test{\n')
        out.write('bool TEST_FAILED = false;\n')
        out.write('std::stringstream TEST_OUT;\n')
        out.write('std::vector<Checkable*> POST_CHECKS;\n')
        out.write('int NUM_KNOWN_ERRORS = 0;\n')

        out.write('} // namespace\n')

        for v in info.extra_globals:
            out.write("%s;\n" % v)

        out.write('\n');

        out.write('TestPlatform get_test_platform(){\n')
        if sys.platform.startswith('linux'):
            out.write('  return TestPlatform::LINUX;\n')
        else:
            out.write('  return TestPlatform::WINDOWS;\n')
        out.write('}\n')
        out.write('\n')

        out.write('std::string g_test_name;\n\n');

        out.write('void set_test_name(const std::string& name){\n')
        out.write('  g_test_name = name;\n')
        out.write('}\n')
        out.write('\n')

        out.write('std::string get_test_name(){\n')
        out.write('  return g_test_name;\n');
        out.write('}\n')
        out.write('\n')

        for f in files:
            out.write('%s\n' % util.file_name_to_declaration(f))
        out.write('\n')

        out.write('std::string find_test_name(int argc, char** argv){\n')
        out.write('  for (int i = 1; i < argc; i++){\n')
        out.write('    if (argv[i][0] != \'-\'){\n')
        out.write('      return argv[i];\n')
        out.write('    }\n')
        out.write('  }\n')
        out.write('  return "";\n');
        out.write('}\n')

        out.write('bool find_silent_flag(int argc, char** argv){\n')
        out.write('  for (int i = 1; i < argc; i++){\n')
        out.write('    if (argv[i] == std::string("--silent")){\n')
        out.write('      return true;\n')
        out.write('    }\n')
        out.write('  }\n')
        out.write('  return false;\n')
        out.write('}\n')

        if len(info.args) != 0:
            out.write('int %s(int argc, char** argv){\n' % main_function_name)
        else:
            out.write('int %s(int, char**){\n' % main_function_name)

        if not sys.platform.startswith('linux'):
            out.write('  SetErrorMode(GetErrorMode()|SEM_NOGPFAULTERRORBOX);\n')
            out.write('  _set_abort_behavior( 0, _WRITE_ABORT_MSG);\n')
            out.write('\n')

        for arg in info.args:
            out.write("%s\n" % arg)

        out.write('  int numFailed = 0;\n')

        for f in files:
            func = util.file_name_to_function_pointer(f)
            info.write_function_call(out, func, f, max_width)

        out.write('  return test::print_test_summary(numFailed);\n')
        out.write('}\n')

    # Create defines.hh
    folder = os.path.split(out_file)[0]
    with open(os.path.join(folder, 'defines.hh'), 'w') as defs:
        if sys.platform.startswith('linux'):
            defs.write('#define TEST_PLATFORM_LINUX\n')
        else:
            defs.write('#define TEST_PLATFORM_WINDOWS\n')


def gen_test_runner(root_dir, out_file):
    gen_runner(root_dir, out_file, test_runner_info())


def gen_bench_runner(root_dir, out_file):
    gen_runner(root_dir, out_file, bench_runner_info())


def gen_image_runner(root_dir, out_file):
    gen_runner(root_dir, out_file, image_runner_info())
