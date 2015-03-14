// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#ifndef FAINT_PY_UGLY_FORWARD_HH
#define FAINT_PY_UGLY_FORWARD_HH
#include <functional>
#include <tuple>
#include "python/py-function-error.hh"
#include "python/py-parse.hh"
#include "bitmap/color.hh"
#include "util/default-constructible.hh"
#include "util/either.hh"
#include "util/plain-type.hh"

namespace faint{

template<typename T>
struct plain_type2type{
  // Loses the const-ness and reference so that parameters can be
  // default constructed before being parsed from PyObject* arguments.
  // Also remaps some types with type2type.
  using type = typename type2type<typename plain_type<T>::type>::type;
};

static PyObject* set_error(const PythonError& err){
  err.SetError();
  return nullptr;
}

// Helper macro for the forwarding structures.
// Resolves the C++-type of "self" (i.e. the first argument for a
// C-Python member function).
//
// Requires a template specialization of MappedType that maps the
// Python object structure to the C++ class.
#define GET_TYPED_SELF(CLASS_T, obj) static_assert(!std::is_same<typename MappedType<CLASS_T>::PYTHON_TYPE, InvalidMappedType>::value, "MappedType not specialized for CLASS_T"); \
  typename MappedType<CLASS_T>::PYTHON_TYPE* self = (typename MappedType<CLASS_T>::PYTHON_TYPE*)obj

// -----------------------------------------------------------------------
// Tuple-unpacking and function call (tuple_unpack)
// -----------------------------------------------------------------------
// Unpack the tuple with parsed arguments and call the C++-function
// Stolen from:
// http://stackoverflow.com/questions/18710537/c-variadic-template-to-replace-typelist
namespace tuple_unpack{
  template<typename RET, typename F, typename Tuple, bool Done, int Total, int... N>
  struct call_impl{
    static PyObject* call(F f, Tuple&& t){
      return call_impl<RET, F, Tuple, Total == 1 + sizeof...(N), Total, N..., sizeof...(N)>::call(f, std::forward<Tuple>(t));
    }
  };

  template<typename RET, typename F, typename Tuple, int Total, int... N>
  struct call_impl<RET, F, Tuple, true, Total, N...>{
    static PyObject* call(F f, Tuple&& t){
      return build_result(f(std::get<N>(std::forward<Tuple>(t))...));
    }
  };

  template<typename F, typename Tuple, int Total, int... N>
  struct call_impl<void, F, Tuple, true, Total, N...>{
    static PyObject* call(F f, Tuple&& t){
      f(std::get<N>(std::forward<Tuple>(t))...);
      return Py_BuildValue("");
    }
  };

  template <typename RET, typename F, typename Tuple>
  PyObject* call(F f, Tuple&& t){
    using ttype = typename std::decay<Tuple>::type;
    return call_impl<RET, F, Tuple, std::tuple_size<ttype>::value == 0, std::tuple_size<ttype>::value>::call(f, std::forward<Tuple>(t));
  }
}

// -----------------------------------------------------------------------
// Structures for recursive argument parsing (ArgParse)
// -----------------------------------------------------------------------
// The static ParseNext function in the ArgParse structs recurse the
// list of argument types and parse one item at a time from PyObject*,
// into a tuple, preparing the C++ function call.

template<class RET, class FUNC, class HEAD, class... Args>
struct ArgParse{
  // Parse arguments recursively. Default construct the "head"-type
  // then parse an item from the Python arguments into it. Put the
  // results in a tuple (to prepare the actual C++ function call) and
  // keep recursin'.
  template<class...PARSED>
  static PyObject* ParseNext(const std::tuple<PARSED...>& parsed, PyObject* pyArgs, int n, int len, const FUNC& f){
    typename plain_type2type<HEAD>::type current;
    if (!parse_item(current, pyArgs, n, len, true)){
      return nullptr;
    }
    else{
      return ArgParse<RET, FUNC, Args...>::ParseNext(std::tuple_cat(parsed, std::tie(current)), pyArgs, n, len, f);
    }
  }
};

template<class RET, class FUNC, class HEAD>
struct ArgParse<RET, FUNC, HEAD>{
  // Parse the last argument
  template<class...PARSED>
  static PyObject* ParseNext(const std::tuple<PARSED...>& t, PyObject* pyArgs, int n, int len, const FUNC& f){
    typename plain_type2type<HEAD>::type current;
    if (!parse_item(current, pyArgs, n, len, true)){
      return nullptr;
    }
    else{
      // Call the C++ function (well, unpack the tuple first...)
      return tuple_unpack::call<RET>(f, std::tuple_cat(t, std::tie(current)));
    }
  }
};

// -----------------------------------------------------------------------
// Entry-points to argument parsing (call_cpp_function)
// -----------------------------------------------------------------------
// The regular call_cpp_function uses ArgParse for recursive argument
// parsing. ArgParse in turn calls the actual function if all goes
// well - otherwise call_cpp_function handles the exception and
// returns a nullptr.
// The function is overloaded for zero argument cases. These do not
// use ArgParse, but call the C++ function immediately.

template<typename Tuple, typename RET, typename FUNC, class HEAD, class... Args>
PyObject* call_cpp_function(PyObject* pyArgs, const FUNC& f, const Tuple& t){
  try{
    int len = static_cast<int>(PySequence_Length(pyArgs)); // Fixme: Check cast
    int n = 0;
    return ArgParse<RET, FUNC, HEAD, Args...>::ParseNext(t, pyArgs, n, len, f);
  }
  catch (const PythonError& error){
    return set_error(error);
  }
  catch (const PresetFunctionError&){
    return nullptr;
  }
}

// Zero-argument method overload (with self as the only content in the tuple)
template<typename Tuple, typename RET, typename FUNC>
PyObject* call_cpp_function(const FUNC& f, const Tuple& t){
  try{
    return tuple_unpack::call<RET>(f, t);
  }
  catch (const PythonError& error){
    return set_error(error);
  }
  catch (const PresetFunctionError&){
    return nullptr;
  }
}

// Free function, zero argument overload
template<typename RET, RET Func()>
PyObject* call_cpp_function(){
  try{
    build_result(Func());
  }
  catch (const PythonError& error){
    return set_error(error);
  }
  catch (const PresetFunctionError&){
    return nullptr;
  }
}

// -----------------------------------------------------------------------
// Forwarding structures ([x]_arg_t)
// -----------------------------------------------------------------------
// A bunch of structures which all contain a function (PythonFunc) follow.
//
// This function:
// 1. makes the wrapped C++-function appear like a
//    PyMethodDef-compatible PyObject*(PyObject*,PyObject*)-function
//
// 2. turns the PyObject*-self into the strongly typed C++-class expected
//    as the first argument of the C++ function
//
// 3. parses the PyObject* argument list into the expected C++-
//    arguments
//
// 4. calls the C++ function if the arguments could be successfully
//    resolved by the wrapper (PythonFunc).
//
// 5. Handles exceptions (and returns nullptr) or turns the strongly
//    typed return into a PyObject*
//
// I couldn't get this to work generically with zero_arguments, hence
// the zero_arg-structs. The ...n_arg_t-variants handle 1-n arguments.

template<typename CLASS_T, typename RET>
struct zero_arg_t{
  // Zero arguments
  template<RET Func(CLASS_T)>
  static PyObject* PythonFunc(PyObject* rawSelf, PyObject* /*args*/){
    GET_TYPED_SELF(CLASS_T, rawSelf);
    if (MappedType<CLASS_T>::Expired(self)){
      MappedType<CLASS_T>::ShowError(self);
      return nullptr;
    }

    CLASS_T cppSelf(MappedType<CLASS_T>::GetCppObject(self));
    auto t(std::tie(cppSelf));
    return call_cpp_function<decltype(t), RET, std::function<RET(CLASS_T)>>(std::function<RET(CLASS_T)>(Func), t);
  }
};

template<typename CLASS_T, typename RET, class... Args>
struct n_arg_t{
  template<RET Func(CLASS_T, Args...)>
  static PyObject* PythonFunc(PyObject* rawSelf, PyObject* args){
    GET_TYPED_SELF(CLASS_T, rawSelf);
    if (MappedType<CLASS_T>::Expired(self)){
      MappedType<CLASS_T>::ShowError(self);
      return nullptr;
    }

    CLASS_T cppSelf(MappedType<CLASS_T>::GetCppObject(self));
    auto t(std::tie(cppSelf));
    return call_cpp_function<decltype(t), RET, std::function<RET(CLASS_T, Args...)>, Args...>
      (args, std::function<RET(CLASS_T,Args...)>(Func), t);
  }
};

template<typename RET>
struct free_zero_arg_t{
  // Zero arguments, free function
  template<RET Func()>
  static PyObject* PythonFunc(PyObject*, PyObject* /*args*/){
    try{
      return build_result(Func());
    }
    catch (const PythonError& error){
      return set_error(error);
    }
    catch (const PresetFunctionError&){
      return nullptr;
    }
  }
};

template<>
struct free_zero_arg_t<void>{
  // Zero arguments, free function, void return type specialization
  template<void Func()>
  static PyObject* PythonFunc(PyObject*, PyObject* /*args*/){
    try{
      Func();
      return Py_BuildValue("");
    }
    catch (const PythonError& error){
      return set_error(error);
    }
    catch (const PresetFunctionError&){
      return nullptr;
    }
  }
};

template<typename RET, class...Args>
struct free_n_arg_t{
  // >0 arguments, free function

  template<RET Func(Args...)>
  static PyObject* PythonFunc(PyObject*, PyObject* args){
    auto t = std::make_tuple();
    return call_cpp_function<decltype(t), RET, std::function<RET(Args...)>, Args...>
      (args, std::function<RET(Args...)>(Func), t);
  }
};

template<typename CLASS_T, typename RET>
struct getter_t{
  template<RET Func(CLASS_T)>
  static PyObject* PythonFunc(PyObject* rawSelf, void*){
    GET_TYPED_SELF(CLASS_T, rawSelf);
    if (MappedType<CLASS_T>::Expired(self)){
      MappedType<CLASS_T>::ShowError(self);
      return nullptr;
    }
    try{
      return build_result(Func(MappedType<CLASS_T>::GetCppObject(self)));
    }
    catch (const PythonError& error){
      return set_error(error);
    }
    catch (const PresetFunctionError&){
      return nullptr;
    }
  }
};

template<typename PY_CLASS_T>
struct init_zero_arg_t{
  // tp_init without arguments
  template<void Func(PY_CLASS_T)>
  static int PythonFunc(typename std::remove_reference<PY_CLASS_T>::type* rawSelf, PyObject* /*args*/, PyObject*){
    auto t(std::tie(*rawSelf));
    PyObject* result = call_cpp_function<decltype(t), void,
      std::function<void(PY_CLASS_T)>>(
        std::function<void(PY_CLASS_T)>(Func), t);
    return result == nullptr? init_fail : init_ok;
  }
};

template<typename PY_CLASS_T, class... Args>
struct init_n_arg_t{
  // tp_init with arguments

  template<void Func(PY_CLASS_T, Args...)>
  static int PythonFunc(typename std::remove_reference<PY_CLASS_T>::type* rawSelf, PyObject* args, PyObject*){
    auto t(std::tie(*rawSelf));
    PyObject* result = call_cpp_function<decltype(t), void, std::function<void(PY_CLASS_T, Args...)>, Args...>
      (args, std::function<void(PY_CLASS_T,Args...)>(Func), t);
    return result == nullptr? init_fail : init_ok;
  }
};

template<typename CLASS_T>
struct repr_t{
  // tp_repr
  template<utf8_string Func(CLASS_T)>
  static PyObject* PythonFunc(PyObject* rawSelf){
    GET_TYPED_SELF(CLASS_T, rawSelf);
    if (MappedType<CLASS_T>::Expired(self)){
      return build_result(MappedType<CLASS_T>::DefaultRepr(self));
    }

    CLASS_T cppSelf(MappedType<CLASS_T>::GetCppObject(self));
    auto t(std::tie(cppSelf));
    return call_cpp_function<decltype(t), utf8_string, std::function<utf8_string(CLASS_T)>>(std::function<utf8_string(CLASS_T)>(Func), t);
  }
};

template<typename CLASS_T, typename T1>
struct setter_t{
  static const int setter_ok = 0;
  static const int setter_fail = -1;

  template<void Func(CLASS_T, T1)>
  static int PythonFunc(PyObject* rawSelf, PyObject* arg, void*){
    GET_TYPED_SELF(CLASS_T, rawSelf);
    if (MappedType<CLASS_T>::Expired(self)){
      MappedType<CLASS_T>::ShowError(self);
      return setter_fail;
    }

    try{
      typename plain_type2type<T1>::type a1;
      int n = 0;
      const int len = static_cast<int>(PySequence_Check(arg) ? PySequence_Length(arg) : 1); // Fixme: Check cast (or change type of len)
      if (!parse_flat(a1, arg, n, len)){
        return setter_fail;
      }
      Func(MappedType<CLASS_T>::GetCppObject(self), a1);
      return setter_ok;
    }
    catch (const PythonError& error){
      set_error(error);
      return setter_fail;
    }
    catch (const PresetFunctionError&){
      return setter_fail;
    }
  }
};

// -----------------------------------------------------------------------
// Declarations for resolving the forwarding structure (resolve)
// -----------------------------------------------------------------------
// Declarations which are used for decltype to resolve the arguments
// of the function pointer passed as argument follow.
//
// These functions are not defined anywhere -- they're only used to
// map function pointers with typed arguments, like
//   void put_pixel(Canvas&, const IntPoint&)
// to the structures above (e.g n_arg_t), which have a regular Python
// signature wrapper member function, PythonFunc, that produces the typed
// arguments by parsing the Python arguments (PyObject*) and then calls
// the real function.

template<typename CLASS_T, typename RET>
zero_arg_t<CLASS_T, RET> resolve(RET(*func)(CLASS_T));

template<typename CLASS_T, class RET, class...Args>
n_arg_t<CLASS_T, RET, Args...> resolve(RET(*func)(CLASS_T, Args...));

template<typename RET>
free_zero_arg_t<RET> free_resolve(RET(*func)());

template<typename RET, class...Args>
free_n_arg_t<RET, Args...> free_resolve(RET(*func)(Args...));

template<typename CLASS_T, typename RET>
getter_t<CLASS_T, RET> get_resolve(RET(*func)(CLASS_T));

template<typename CLASS_T, typename T1>
setter_t<CLASS_T, T1> set_resolve(void(*func)(CLASS_T, T1));

template<typename PY_CLASS_T, class...Args>
init_n_arg_t<PY_CLASS_T, Args...> init_resolve(void(*func)(PY_CLASS_T, Args...));

template<typename PY_CLASS_T>
init_zero_arg_t<PY_CLASS_T> init_resolve(void(*func)(PY_CLASS_T));

template<typename PY_CLASS_T>
repr_t<PY_CLASS_T> repr_resolve(utf8_string(*func)(PY_CLASS_T));

// Presumably necessary wrapping since adding scope resolution after
// decltype(...) didn't compile
template<typename ForwardingStruct>
struct Route{
  using T = ForwardingStruct;
};

// -----------------------------------------------------------------------
// Macros for use in PyMethodDef:s (FORWARDER etc.)
// -----------------------------------------------------------------------
// Macros for use in PyMethodDef arrays. These are the entry points to
// the function wrapping.
//
// The macros wrap a C++-function with known argument types in the
// guise of a PyObject*(PyObject*, PyObject*).
//
// The C++ function will only be called if the arguments can be parsed
// into the required types (see py-parse.hh).
//
// Explanation:
//  1. Accepts a C++-function (the CppFunc argument)
//
//  2. uses the "resolve"-overloads to resolve a
//     template-instantiation of the forwarding structure templates
//
//  3. nests the resolved struct in Route, since decltype(...)::PythonFunc
//     wouldn't compile
//
//  4. specializes PythonFunc with the C++-function pointer so that the
//     function can be called (the resolve-step only gave the signature,
//     not the address).

// For methods (taking a self-object as the first argument)
#define FORWARDER(CppFunc, ARGS_TYPE, NAME, DOC){NAME, Route<decltype(resolve(CppFunc))>::T::PythonFunc<CppFunc>, ARGS_TYPE, DOC}

// For free functions (no self-object))
#define FREE_FORWARDER(CppFunc, ARGS_TYPE, NAME, DOC){NAME, Route<decltype(free_resolve(CppFunc))>::T::PythonFunc<CppFunc>, ARGS_TYPE, DOC}

// For getters
#define GETTER_FORWARDER(CppFunc)Route<decltype(get_resolve(CppFunc))>::T::PythonFunc<CppFunc>

// For setters
#define SETTER_FORWARDER(CppFunc)Route<decltype(set_resolve(CppFunc))>::T::PythonFunc<CppFunc>

// For getters and setters
#define PROPERTY_FORWARDER(CppStruct, NAME, DOC){(char*)NAME, GETTER_FORWARDER(CppStruct::Get), SETTER_FORWARDER(CppStruct::Set), (char*)DOC, nullptr}

// For Python-class initialization functions (tp_init)
#define INIT_FORWARDER(CppFunc)(initproc)Route<decltype(init_resolve(CppFunc))>::T::PythonFunc<CppFunc>

// For tp_repr
#define REPR_FORWARDER(CppFunc)(reprfunc)Route<decltype(repr_resolve(CppFunc))>::T::PythonFunc<CppFunc>

} // namespace

#endif
