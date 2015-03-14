// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#ifndef FAINT_TEST_SYS_HH
#define FAINT_TEST_SYS_HH
#include "test-sys/test.hh"
#include "test-sys/test-name.hh"

void verify_post_checks(){
  for (Checkable* check : POST_CHECKS){
    check->Check();
    delete check;
  }
  POST_CHECKS.clear();
}

// Fixme: Move to own file (like run-bench.hh et al.)
void run_test(void (*func)(), const std::string& fileName, int max_w, int& numFailed, bool silent){
  // Test title
  const std::string name = fileName.substr(0, fileName.size() - 4);
  set_test_name(name);
  if (!silent){
    std::cout << name << ":" << std::string(static_cast<size_t>(max_w + 1) - name.size(), ' ') << std::flush;
  }

  // Run the test
  try{
    func();
  }
  catch(const AbortTestException&){
    // Test will already have been flagged as failed.
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

int print_test_summary(const int numFailed){
  if (numFailed > 0){
    std::cout << std::endl;
    if (NUM_KNOWN_ERRORS > 0){
      std::cout << "Known errors: " << NUM_KNOWN_ERRORS << std::endl;
    }
    std::cout << "Error: " << numFailed << " " <<
      (numFailed == 1 ? "test" : "tests") << " failed!" << std::endl;
    return 1;
  }
  if (NUM_KNOWN_ERRORS > 0){
    std::cout << "Known errors: " << NUM_KNOWN_ERRORS << std::endl;
  }
  return 0;
}

#endif
