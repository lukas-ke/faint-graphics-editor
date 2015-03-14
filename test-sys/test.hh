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
#include <cmath>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

extern std::stringstream TEST_OUT;
extern bool TEST_FAILED;
extern int NUM_KNOWN_ERRORS;

class AbortTestException{};

// Add braces around the arguments, to get past preprocessor not
// understanding initializer-lists.
#define LIST(...){__VA_ARGS__}

#define MESSAGE(MSG) TEST_OUT << "  Message(" << __LINE__ << ") " << MSG << std::endl;

#define TIME_MESSAGE(MSG) TEST_OUT << "  " << MSG << std::endl;

template<typename T1, typename T2>
std::string str_not_equal_hex(T1 v1, T2 v2){
  std::stringstream ss;
  ss << std::hex;
  ss << static_cast<unsigned int>(v1) << " != " << static_cast<unsigned int>(v2);
  return ss.str();
}

#define EQUAL_HEX(A,B) if ((A) != (B)){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << str_not_equal_hex(A, B) << ")" << std::endl; TEST_FAILED=true;}

#define EQUAL(A,B) if (!((A) == (B))){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << A << " != " << B << ")" << std::endl; TEST_FAILED=true;}

#define EQUALF(A,B,FMT)if (!((A) == (B))){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << FMT(A) << " != " << FMT(B) << ")" << std::endl; TEST_FAILED=true;}

#define NOT_EQUAL(A,B) if ((A) == (B)){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " == " << #B << " (" << A << " == " << B << ")" << std::endl; TEST_FAILED=true;}

#define ASSERT_EQUAL(A,B) if ((A) != (B)){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << A << " != " << B << ")" << std::endl; TEST_FAILED=true; throw AbortTestException();}

class Epsilon{
public:
  explicit constexpr Epsilon(double v)
    : value(v)
  {}

  operator double() const{
    return value;
  }

  double value;
};

namespace test{

// Helper to make the static_assert dependent on T.
template<typename T>
struct TypeDependentFalse{
  static const bool value = false;
};


template<typename T>
double to_double(const T&){
  static_assert(TypeDependentFalse<T>::value,
    "to_double not overloaded for type.");
  return 0.0;
}

inline double to_double(double v){
  return v;
}

} // namespace

template<typename T>
bool test_near(const T& a, const T& b, const Epsilon& e){
  return std::fabs(test::to_double(a) - test::to_double(b)) < e;
}

constexpr Epsilon operator "" _eps(long double v){
  return Epsilon(static_cast<double>(v));
}

#define NEAR(A,B,EPS) if (!test_near(A, B, EPS)){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " != " << #B << " (" << A << " != " << B << ")" << std::endl; TEST_FAILED=true;}


#define VERIFY(C) if ((C)){}else{ TEST_OUT << "  Error(" << __LINE__ << "): " << #C << " failed." << std::endl; TEST_FAILED=true;}

#define NOT(C) if ((!C)){}else{ TEST_OUT << "  Error(" << __LINE__ << "): NOT " << #C << " failed." << std::endl; TEST_FAILED=true;}

#define ASSERT(C) if ((C)){}else{ TEST_OUT << "  Error(" << __LINE__ << "): " << #C << " failed." << std::endl; TEST_FAILED=true; throw AbortTestException();}

#define ABORT_IF(C) if (!(C)){}else{ TEST_OUT << "  Error(" << __LINE__ << "): " << #C << " failed." << std::endl; TEST_FAILED=true; throw AbortTestException();}

inline void fail_test(){}

inline void fail_test(const char* message){
  TEST_OUT << "  " << message << std::endl;
  fail_test();
}

template<typename ...T>
void fail_test(const char* message, const T&... args){
  TEST_OUT << message << " ";
  fail_test(args...);
}

// TODO: Rename to e.g. ABORT_TEST
#define FAIL(...) TEST_OUT << "  FAIL triggered on line " << __LINE__ << std::endl; TEST_FAILED=true; fail_test(__VA_ARGS__); throw AbortTestException();

#define SET_FAIL() TEST_OUT << "  SET_FAIL triggered on line " << __LINE__ << std::endl; TEST_FAILED=true

#define KNOWN_ERROR(C) if ((C)){TEST_OUT << "  Error(" << __LINE__ << "): KNOWN_ERROR \"" << #C << "\"..\n" << "  ..evaluated OK. Test not updated?" << std::endl; TEST_FAILED=true;}else{ TEST_OUT << "  Known error(" << __LINE__ << "): " << #C << std::endl; NUM_KNOWN_ERRORS += 1;}

#define KNOWN_INEQUAL(A,B) if ((A) == (B)){ TEST_OUT << "  Error(" << __LINE__ << "): " << #A << " == " << #B << " (" << A << " != " << B << ")" << " ..evaluated OK. Test not updated?" << std::endl; TEST_FAILED=true;}else{ TEST_OUT << "  Known error(" << __LINE__ << "): " << A << "!=" << B << std::endl; NUM_KNOWN_ERRORS += 1;}

struct FailIfCalled{
  FailIfCalled(int line, bool abort=false)
    : m_line(line),
      m_abort(abort)
  {}

  template<typename... Args>
  void operator()(Args...) const{
    TEST_OUT << "  FailIfCalled was called on line " << m_line << std::endl;
    TEST_FAILED = true;
    if (m_abort){
      throw AbortTestException();
    }
  }
  int m_line;
  bool m_abort;
};

class Checkable{
public:
  virtual ~Checkable() = default;
  virtual bool Check() const = 0;
};

extern std::vector<Checkable*> POST_CHECKS;


class FailUnlessCalled : public Checkable{
public:
  FailUnlessCalled(int line)
    : m_called(false),
      m_line(line)
  {}

  template<typename... Args>
  void operator()(Args...) const {
    m_called = true;
  }

  bool Check() const override{
    if (!m_called){
      TEST_OUT << "  FAIL_UNLESS_CALLED on line " << m_line << " was not called." << std::endl;
      TEST_FAILED = true;
    }
    return m_called;
  }
private:
  mutable bool m_called;
  int m_line;
};

#define FWD(CALL);{try{bool alreadyFailed = TEST_FAILED;CALL;if (TEST_FAILED && !alreadyFailed){TEST_OUT << " ... called via FWD on line " << __LINE__ << std::endl;}}catch(const AbortTestException&){TEST_OUT << " ... called via FWD on line " << __LINE__ << std::endl; throw;}}


template<typename FUNC>
class FailUnlessCalledFwd : public Checkable{
public:
  FailUnlessCalledFwd(FUNC fwd, int line)
    : m_called(false),
      m_line(line),
      m_fwd(fwd)
  {}

  template<typename... Args>
  void operator()(Args...args) const {
    m_called = true;
    bool alreadyFailed = TEST_FAILED;
    m_fwd(args...);
    if (TEST_FAILED && !alreadyFailed){
      TEST_OUT << "  ... called via FAIL_UNLESS_CALLED_FWD on line " << m_line << std::endl;
    }
  }

  bool Check() const override{
    if (!m_called){
      TEST_OUT << "  FAIL_UNLESS_CALLED on line " << m_line << " was not called." << std::endl;
      TEST_FAILED = true;
    }
    return m_called;
  }
private:
  mutable bool m_called;
  int m_line;
  FUNC m_fwd;
};

template<typename FUNC>
class FailIfCalledFwd : public Checkable{
public:
  FailIfCalledFwd(FUNC fwd, int line)
    : m_called(false),
      m_line(line),
      m_fwd(fwd)
  {}

  template<typename... Args>
  void operator()(Args...args) const {
    m_called = true;
    bool alreadyFailed = TEST_FAILED;
    m_fwd(args...);
    if (TEST_FAILED && !alreadyFailed){
      TEST_OUT << "  ... called via FAIL_IF_CALLED_FWD on line " << m_line << std::endl;
    }
  }

  bool Check() const override{
    if (m_called){
      TEST_OUT << "  FAIL_IF_CALLED on line " << m_line << " was called." << std::endl;
      TEST_FAILED = true;
    }
    return m_called;
  }
private:
  mutable bool m_called;
  int m_line;
  FUNC m_fwd;
};


inline FailUnlessCalled& create_fail_unless_called(int line){
  FailUnlessCalled* f = new FailUnlessCalled(line);
  POST_CHECKS.push_back(f);
  return *f;
}

template<typename FUNC>
inline FailUnlessCalledFwd<FUNC>& create_fail_unless_called_fwd(FUNC func,
  int line)
{
  FailUnlessCalledFwd<FUNC>* f = new FailUnlessCalledFwd<FUNC>(func, line);
  POST_CHECKS.push_back(f);
  return *f;
}

template<typename FUNC>
inline FailIfCalledFwd<FUNC>& create_fail_if_called_fwd(FUNC func,
  int line)
{
  FailIfCalledFwd<FUNC>* f = new FailIfCalledFwd<FUNC>(func, line);
  POST_CHECKS.push_back(f);
  return *f;
}

#define FAIL_IF_CALLED()(FailIfCalled(__LINE__))
#define FAIL_UNLESS_CALLED()(create_fail_unless_called(__LINE__))
#define FAIL_UNLESS_CALLED_FWD(FUNC)(create_fail_unless_called_fwd(FUNC, __LINE__))
#define FAIL_IF_CALLED_FWD(FUNC)(create_fail_if_called_fwd(FUNC, __LINE__))

enum class TestPlatform{
  LINUX,
  WINDOWS
};

TestPlatform get_test_platform();

#endif
