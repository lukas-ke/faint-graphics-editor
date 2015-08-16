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

#ifndef FAINT_TEST_INTERNAL_HH
#define FAINT_TEST_INTERNAL_HH
#include <cmath>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace test{

extern std::stringstream TEST_OUT;
extern bool TEST_FAILED;
extern int NUM_KNOWN_ERRORS;

inline bool write_error(const char* file, int line, const std::string& text){
  // Error-syntax recognized by emacs Compilation mode
  TEST_OUT << file << "(" << line << "): error: " << text << std::endl;

  TEST_FAILED = true;
  return false;
}

class AbortTestException{};

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

template<typename T>
bool test_near(const T& a, const T& b, const Epsilon& e){
  return std::fabs(test::to_double(a) - test::to_double(b)) < e;
}

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

inline bool abort_test(const char* file, int line, const std::string& text){
  write_error(file, line, text);
  TEST_FAILED = true;
  throw AbortTestException();
}

template<typename T1, typename T2>
std::string str_not_equal_hex(T1 v1, T2 v2){
  std::stringstream ss;
  ss << std::hex;
  ss << static_cast<unsigned int>(v1) << " != " << static_cast<unsigned int>(v2);
  return ss.str();
}

template<typename T1, typename T2>
std::string str_cmp_values(T1 v1, const char* cmp, T2 v2){
  std::stringstream ss;
  ss << v1 << " " << cmp << " " << v2;
  return ss.str();
}

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
      TEST_OUT << "  FAIL_UNLESS_CALLED on line " << m_line <<
        " was not called." << std::endl;
      TEST_FAILED = true;
    }
    return m_called;
  }
private:
  mutable bool m_called;
  int m_line;
};

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
      TEST_OUT << "  ... called via FAIL_UNLESS_CALLED_FWD on line " <<
        m_line << std::endl;
    }
  }

  bool Check() const override{
    if (!m_called){
      TEST_OUT << "  FAIL_UNLESS_CALLED on line " << m_line <<
        " was not called." << std::endl;
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
      TEST_OUT << "  ... called via FAIL_IF_CALLED_FWD on line "
               << m_line << std::endl;
    }
  }

  bool Check() const override{
    if (m_called){
      TEST_OUT << "  FAIL_IF_CALLED on line " << m_line <<
        " was called." << std::endl;
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

} // namespace

#endif
