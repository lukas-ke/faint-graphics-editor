// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
//
// Licensed under the Apache License, Version 2.0 (the "License"); you
// may not use this file except in compliance with the License. You
// may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
// implied. See the License for the specific language governing
// permissions and limitations under the License.

#ifndef FAINT_RUN_TEST_HH
#define FAINT_RUN_TEST_HH
#include <stdexcept>

namespace test{

void fail_on_exception(const std::string& type, const std::string& what){
  TEST_FAILED = true;
  TEST_OUT << "  Exception: \"" << what << "\" " << "(" << type << ")" <<
    std::endl;
}

void run_test(void (*func)(),
  const std::string& fileName,
  int max_w,
  int& numFailed,
  bool silent)
{
  // Test title
  const std::string name = fileName.substr(0, fileName.size() - 4);
  set_test_name(name);
  if (!silent){
    std::cout << name << ":" << std::string(static_cast<size_t>(max_w + 1) -
      name.size(), ' ') << std::flush;
  }

  // Run the test
  try{
    func();
  }
  catch (const AbortTestException&){
    // Test will already have been flagged as failed.
  }
  catch (const std::out_of_range& e){
    fail_on_exception("std::out_of_range", e.what());
  }
  catch (const std::logic_error& e){
    fail_on_exception("std::logic_error", e.what());
  }
  catch (const std::runtime_error& e){
    fail_on_exception("std::runtime_error", e.what());
  }
  catch (const std::exception& e){
    fail_on_exception("std::exception", e.what());
  }

  verify_post_checks();

  // Append result to title
  if (!silent){
    std::cout <<(TEST_FAILED ? "FAIL" : "ok") << std::endl;
  }

  if (TEST_FAILED){
    numFailed += 1;
    if (silent){
      // Heading was not printed when silent.
      std::cout << name << ": FAIL" << std::endl;
    }
    std::cout << TEST_OUT.str();
    TEST_FAILED = false;
  }
  else{
    if (!silent){
      std::cout << TEST_OUT.str();
    }
  }
  TEST_OUT.str("");
}

} // namespace

#endif
