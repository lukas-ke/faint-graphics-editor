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

#ifndef FAINT_TEST_HH
#define FAINT_TEST_HH
#include "test-sys/test-internal.hh"

// Literal-suffix for creating an epsilon (e.g. for NEAR).
constexpr test::Epsilon operator "" _eps(long double v){
  return test::Epsilon(static_cast<double>(v));
}

enum class TestPlatform{
  LINUX,
  WINDOWS
};

TestPlatform get_test_platform();

// Add braces around the arguments, to get past preprocessor not
// understanding initializer-lists.
#define LIST(...){__VA_ARGS__}

// Add a message to the text output (including the line number)
#define MESSAGE(MSG) test::TEST_OUT << "  Message(" << __LINE__ << ") " << MSG << std::endl;

// If the two values are inequal the test is marked as failed and the
// values are output in hex-representation.
#define EQUAL_HEX(A,B) if ((A) != (B)){ test::TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << test::str_not_equal_hex(A, B) << ")" << std::endl; test::TEST_FAILED=true;}

// Marks the test as failed if the values are different.
#define EQUAL(A,B) (((A) == (B)) ? true : test::write_error(__FILE__, __LINE__, #A " != " #B " (" + test::str_cmp_values(A, "!=", B) + ")"))

// Same as EQUAL, but takes a formatter argument which is applied to
// both values for the error output.
#define EQUALF(A,B, FMT) (((A) == (B)) ? true : test::write_error(__FILE__, __LINE__, #A " != " #B " (" + test::str_cmp_values(FMT(A), "!=", FMT(B)) + ")"))

// Mark the test as failed if the values are equal.
#define NOT_EQUAL(A,B) (((A) == (B)) ? test::write_error(__FILE__, __LINE__, #A " == " #B " (" + test::str_cmp_values(A, "==", B) + ")") : true)

// Aborts the test if if the values are different.
#define ASSERT_EQUAL(A,B) (((A) == (B)) ? true : test::abort_test(__FILE__, __LINE__, #A " != " #B " (" + test::str_cmp_values(A, "!=", B) + ")"))

// Equality check with epsilon. Marks the test as failed if the values
// are not equal within the given EPS.
#define NEAR(A,B,EPS) if (!test_near(A, B, EPS)){ test::TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << A << " != " << B << ")" << std::endl; test::TEST_FAILED=true;}

// Mark the test as failed if the condition is false.
#define VERIFY(C) ((C) ? true : test::write_error(__FILE__, __LINE__, "VERIFY " #C))

// Mark the test as failed if the condition is true.
#define NOT(C) ((!C) ? true : test::write_error(__FILE__, __LINE__, "NOT " #C))

// Abort the test if the condition is false.
#define ASSERT(C) ((C) ? true : test::abort_test(__FILE__, __LINE__, "ASSERT "#C))

// Abort the test if the condition is true.
#define ABORT_IF(C) (!(C) ? true : test::abort_test(__FILE__, __LINE__, "ABORT_IF " #C))

// Abort the test and output any string arguments.
#define ABORT_TEST(...) test::TEST_OUT << "  FAIL triggered on line " << __LINE__ << std::endl; test::TEST_FAILED=true; test::output_all(__VA_ARGS__); throw test::AbortTestException();

// Mark the test as failed
#define SET_FAIL() test::TEST_OUT << "  SET_FAIL triggered on line " << __LINE__ << std::endl; test::TEST_FAILED=true

// Indicates a known error if C evaluates to false. This will not be
// treated as a unit test failure, and allows highlighting "long-term"
// problems in a less disruptive way than failed tests.
//
// The known errors are summarized after the test run, without
// causing a non-zero exit status.
//
// Note: If C evaluates to true, this indicates that the known error
// has been fixed, and the check should be changed to a VERIFY. The
// test will in this case be marked as failed.
#define KNOWN_ERROR(C) if ((C)){test::TEST_OUT << "  Error(" << __LINE__ << "): KNOWN_ERROR \"" << #C << "\"..\n" << "  ..evaluated OK. Test not updated?" << std::endl; test::TEST_FAILED=true;}else{ test::TEST_OUT << "  Known error(" << __LINE__ << "): " << #C << std::endl; test::NUM_KNOWN_ERRORS += 1;}

// Similar to known error, but for a known value difference.
// In this case, it is a known error that A is different from B.
#define KNOWN_INEQUAL(A,B) if ((A) == (B)){ test::TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " == " << #B << " (" << A << " != " << B << ")" << " ..evaluated OK. Test not updated?" << std::endl; test::TEST_FAILED=true;}else{ test::TEST_OUT << "  Known error(" << __LINE__ << "): " << A << "!=" << B << std::endl; test::NUM_KNOWN_ERRORS += 1;}

// Wrapper for function calls so that the call site can be printed in
// addition to a failure within a function.
#define FWD(CALL);{try{bool alreadyFailed = test::TEST_FAILED;CALL;if (test::TEST_FAILED && !alreadyFailed){test::TEST_OUT << " ... called via FWD on line " << __LINE__ << std::endl;}}catch(const test::AbortTestException&){test::TEST_OUT << " ... called via FWD on line " << __LINE__ << std::endl; throw;}}

// Returns a functor which, if called, will mark the test as failed.
// The line with FAIL_IF_CALLED will be indicated in the error.
#define FAIL_IF_CALLED()(test::FailIfCalled(__LINE__))

// Returns a functor which will mark the test as failed at the end of
// the test if it was not called at least once during the test. This
// is evaluated after the test code is completed. The line with
// FAIL_UNLESS_CALLED will be indicated in the error.
#define FAIL_UNLESS_CALLED()(test::create_fail_unless_called(__LINE__))

// Variant of FAIL_UNLESS_CALLED which when called, delegates to the
// specified function. This allows adding additional checks on the
// passed arguments, in addition to ensuring the additional function
// gets called at all.
#define FAIL_UNLESS_CALLED_FWD(FUNC)(test::create_fail_unless_called_fwd(FUNC, __LINE__))

// Variant of FAIL_IF_CALLED, which in addition to marking the test as
// failed, forwards to the specified function. Typically used for
// printing the arguments to provide more error information.
#define FAIL_IF_CALLED_FWD(FUNC)(test::create_fail_if_called_fwd(FUNC, __LINE__))

#endif
