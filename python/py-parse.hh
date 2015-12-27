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

#ifndef FAINT_PY_PARSE_HH
#define FAINT_PY_PARSE_HH
#include <map>
#include "geo/range.hh"
#include "python/bound.hh"
#include "python/bound-object.hh"
#include "python/py-fwd.hh"
#include "python/py-bitmap.hh"
#include "python/py-function-error.hh"
#include "text/formatting.hh" // for str_int
#include "util/common-fwd.hh"
#include "text/utf8-string.hh"
#include "util/default-constructible.hh"
#include "util/delay.hh"
#include "util/either.hh"
#include "util/type-dependent-false.hh"

namespace faint{

template<typename T>
bool parse_item(T& item, PyObject* args, Py_ssize_t& n, Py_ssize_t len, bool allowFlat);

// Remap some types to support parsing (e.g. for types that are not
// default-constructible.
template<typename T>
struct type2type{
  // By default, types map to themselves.
  using type = T;
};

// Default-constructible remapping for Either
template<typename T1, typename T2> struct type2type<Either<T1, T2>>{
  using type = DefaultConstructible<Either<T1,T2> >;
};

// Default constructible remapping for FilePath
template<> struct type2type<FilePath>{
  using type = DefaultConstructible<FilePath>;
};

// Default constructible remapping for FilePath
template<> struct type2type<Angle>{
  using type = DefaultConstructible<Angle>;
};

// Default constructible remapping for FilePath
template<> struct type2type<Delay>{
  using type = DefaultConstructible<Delay>;
};

class ObjRaster;
class ObjText;

template<typename T>
struct arg_traits{};

template<> struct arg_traits<bitmapObject> {static const TypeName name;};
template<> struct arg_traits<Canvas> {static const TypeName name;};
template<> struct arg_traits<coord>{static const TypeName name;};
template<> struct arg_traits<BoundObject<Object>>{static const TypeName name;};
template<> struct arg_traits<BoundObject<ObjRaster>>{static const TypeName name;};
template<> struct arg_traits<BoundObject<ObjText>>{static const TypeName name;};
template<> struct arg_traits<Grid> {static const TypeName name;};
template<> struct arg_traits<Image> {static const TypeName name;};
template<> struct arg_traits<Index>{static const TypeName name;};
template<> struct arg_traits<IntLineSegment>{static const TypeName name;};
template<> struct arg_traits<LineSegment>{static const TypeName name;};
template<> struct arg_traits<Object> {static const TypeName name;};
template<> struct arg_traits<Object*> {static const TypeName name;};
template<> struct arg_traits<Paint>{static const TypeName name;};
template<> struct arg_traits<Settings>{static const TypeName name;};
template<> struct arg_traits<Tri>{static const TypeName name;};
template<> struct arg_traits<utf8_string>{static const TypeName name;};

bool parse_int(PyObject*, Py_ssize_t n, int* value);
bool parse_coord(PyObject*, Py_ssize_t n, coord* value);

// Parse flat for Faint Objects
template<typename T>
bool parse_flat(BoundObject<T>& obj, PyObject* args, Py_ssize_t& n,
  Py_ssize_t len)
{
  BoundObject<Object> rawObj;
  bool ok = parse_flat(rawObj, args, n, len);
  if (!ok){
    return false;
  }

  T* typedObj = dynamic_cast<T*>(rawObj.obj);
  if (typedObj == nullptr){
    throw TypeError(arg_traits<BoundObject<T>>::name, n);
  }
  obj = BoundObject<T>(rawObj.ctx, rawObj.canvas, typedObj, rawObj.frameId);
  return true;
}

// parse_flat for Either
template<typename T1,typename T2>
bool parse_item(DefaultConstructible<Either<T1, T2> >& obj, PyObject* args, Py_ssize_t& n, Py_ssize_t len, bool allowFlat){
  try{
    typename type2type<T1>::type first;
    if (parse_item(first, args, n, len, allowFlat)){ // Fixme: allow flat?
      obj.Set((T1)first);
      return true;
    }
    else{
      PyErr_Clear();
    }
  }
  catch(const ValueError&){} // Try T2 too
  catch(const TypeError&){} // Try T2 too
  catch(const PresetFunctionError&){
    PyErr_Clear();
  } // Try T2 too

  typename type2type<T2>::type second;
  if (parse_item(second, args, n, len, allowFlat)){ // Fixme: allow flat?
    obj.Set((T2)second);
    return true;
  }
  return false;
}
bool parse_flat(Object*&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(DefaultConstructible<Angle>&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(DefaultConstructible<Delay>&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(bitmapObject*&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(BoundObject<Object>&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Calibration&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Canvas*&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(ColorSpan&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Grid&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(DefaultConstructible<FilePath>&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(const Image*&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(bool&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(int&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Index&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Tri&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(coord&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(IntRect&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(utf8_string&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(std::string&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Settings&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Bitmap&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(AngleSpan&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(KeyPress& value, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(IntSize&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Color&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(ColorStop&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Paint&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(IntLineSegment&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(IntPoint&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(LineSegment&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Point&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(ColRGB&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Rect&, PyObject*, Py_ssize_t& n, Py_ssize_t len);
bool parse_flat(Radii&, PyObject*, Py_ssize_t& n, Py_ssize_t len);

template<typename T, typename category, int ID>
bool parse_flat(Subtype<T, category, ID>& obj, PyObject* args, Py_ssize_t& n, Py_ssize_t len){
  using base_type = typename Subtype<T, category, ID>::base_type;
  return parse_flat(static_cast<base_type&>(obj), args, n, len);
}

// Parse flat for vectors (sequences)
template<typename T>
bool parse_flat(std::vector<T>& vec, PyObject* args, Py_ssize_t& n, Py_ssize_t len){
  if (len - n < 0){
    throw TypeError("Too few arguments for parsing sequence of " +
      arg_traits<T>::name.Get());
  }

  if (PyUnicode_Check(args)){
    throw TypeError(comma_sep(space_sep("Expected sequence of",
      arg_traits<T>::name.Get()), "got string"));
  }
  if (!PySequence_Check(args)){
    throw TypeError("Not a sequence");
  }

  std::vector<T> temp;
  for (auto i = n; i < len; i++){
    T item;
    auto n2 = i;
    if (!parse_item(item, args, n2, len, true)){
      return false;
    }
    temp.push_back(item);
  }
  n += resigned(temp.size());
  vec = temp;
  return true;
}

// Parse flat for pairs
template<typename T1, typename T2>
bool parse_flat(std::pair<T1, T2>& pair,
  PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len)
{
  if (len - n < 0){
    throw TypeError(space_sep("Too few arguments for parsing pair of",
      comma_sep(arg_traits<T1>::name.Get(), arg_traits<T2>::name.Get())));
  }

  if (PyUnicode_Check(args)){
    throw TypeError(space_sep(
      Sentence("Expected pair of",
        comma_sep(arg_traits<T1>::name.Get(), arg_traits<T2>::name.Get())),
      " - Got string."));
  }

  T1 v1;
  auto i0 = n;
  if (!parse_item(v1, args, i0, len, true)){
    return false;
  }
  T2 v2;
  auto i1 = n + 1;
  if (!parse_item(v2, args, i1, len, true)){
    return false;
  }
  pair.first = v1;
  pair.second = v2;
  n += 2;
  return true;
}

// Parse flat for maps (dictionaries)
template<typename T1, typename T2>
bool parse_item(std::map<T1, T2>& theMap, PyObject* args,
  Py_ssize_t& n, Py_ssize_t len)
{
  if (len - n < 0){
    throw TypeError(space_sep("Too few arguments for parsing dict of",
      comma_sep(arg_traits<T1>::name.Get(), arg_traits<T2>::name.Get())));
  }

  if (!PyDict_Check(args)){
    throw TypeError("Not a dictionary");
  }

  std::map<T1, T2> temp;
  scoped_ref items(PyDict_Items(args));
  auto nPairs = PySequence_Length(items.get());

  for (decltype(nPairs) i = 0; i != nPairs; i++){
    PyObject* pair = PySequence_GetItem(items.get(), i);

    Py_ssize_t n2 = 0;
    T1 key;
    T2 value;
    if (!parse_item(key, pair, n2, 2, true)){
      return false;
    }
    if (n2 != 1){
      return false;
    }
    if (!parse_item(value, pair, n2, 2, true)){
      return false;
    }
    temp[key] = value;
  }
  n += 1;
  theMap = temp;
  return true;
}

// Parse flat for maps (dictionaries)
template<typename T1, typename T2>
bool parse_flat(std::map<T1, T2>& theMap, PyObject* args,
  Py_ssize_t& n, Py_ssize_t len)
{
  if (len - n < 0){
    throw TypeError(space_sep("Too few arguments for parsing dict of",
      comma_sep(arg_traits<T1>::name.Get(), arg_traits<T2>::name.Get())));
  }

  scoped_ref dict(PySequence_GetItem(args, n));
  if (!PyDict_Check(dict.get())){
    throw TypeError("Not a dictionary");
  }

  std::map<T1, T2> temp;
  scoped_ref items(PyDict_Items(dict.get()));
  auto nPairs = PySequence_Length(items.get());

  for (decltype(nPairs) i = 0; i != nPairs; i++){
    PyObject* pair = PySequence_GetItem(items.get(), i);

    Py_ssize_t n2 = 0;
    T1 key;
    T2 value;
    if (!parse_item(key, pair, n2, 2, true)){
      return false;
    }
    if (n2 != 1){
      return false;
    }
    if (!parse_item(value, pair, n2, 2, true)){
      return false;
    }
    temp[key] = value;
  }
  n += 1;
  theMap = temp;
  return true;
}

template<int MIN_BOUND, int MAX_BOUND>
bool parse_flat(StaticBoundedInt<MIN_BOUND, MAX_BOUND>& value, PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len)
{
  int raw;
  if (!parse_flat(raw, args, n, len)){
    return false;
  }

  using value_type = StaticBoundedInt<MIN_BOUND, MAX_BOUND>;

  if (!value_type::Valid(raw)){
    ClosedIntRange range(as_closed_range<value_type>());
    throw ValueError(space_sep("Value", str_int(raw),
        "outside valid range", str_range(range)));
  }
  value = value_type(raw);
  // n was incremented by parse_flat(int&...)
  return true;
}

template<int MIN_BOUND, int MAX_BOUND>
bool parse_flat(StaticBoundedInterval<MIN_BOUND, MAX_BOUND>& range,
  PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len)
{
  if (len - n < 2){
    throw ValueError("Too few arguments for parsing range");
  }

  using def_range_t = StaticBoundedInterval<MIN_BOUND, MAX_BOUND>;

  int low, high;
  if (!parse_int(args, n, &low) ||
    !parse_int(args, n+1, &high)){
    return false;
  }

  n += 2;

  Interval interval(make_interval(low, high));
  if (!def_range_t::Valid(interval)){
    throw ValueError(space_sep("Threshold of", str_interval(interval),
      "outside valid range",
      str_range(as_closed_range<def_range_t>())));
  }
  range = StaticBoundedInterval<MIN_BOUND, MAX_BOUND>(interval);
  return true;
}

template<typename T>
bool unary_item(const T&){
  return false;
}

inline bool unary_item(const Bitmap&){
  return true;
}

inline bool unary_item(const Canvas*&){
  return true;
}

inline bool unary_item(bitmapObject*&){
  return true;
}

inline bool unary_item(const std::string&){
  return true;
}

inline bool unary_item(const FilePath&){
  return true;
}

inline bool unary_item(int){
  return true;
}

inline bool unary_item(coord){
  return true;
}

inline bool unary_item(bool){
  return true;
}

// Assigns the args parameter to the item parameter and returns true.
// This allows forwarding with Python args list.
// (Strongly typed target functions should be preferred).
bool parse_item(PyObject*& item,
  PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len,
  bool);

// Parse item number n from the args sequence with length len
// Store the result in item.
template<typename T>
bool parse_item(T& item, PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len,
  bool allowFlat)
{
  if (len - n < 1){
    throw ValueError("Incorrect number of arguments");
  }

  scoped_ref elems(PySequence_GetItem(args, n));
  if (PySequence_Check(elems.get())){
    Py_ssize_t nOther = 0;

    bool ok = parse_flat(item,
      elems.get(),
      nOther,
      PySequence_Length(elems.get()));

    if (!ok){
      return false;
    }
    n += 1;
    return true;
  }
  else if (allowFlat || unary_item(item)){
    return parse_flat(item, args, n, len);
  }
  throw TypeError("Must be a sequence");
}

inline bool is_none(PyObject* args, Py_ssize_t n, Py_ssize_t len){
  if (len <= n){
    return false;
  }
  scoped_ref obj(PySequence_GetItem(args, n));
  return obj.get() == Py_None;
}

// Parses the argument at n into the Optional item if available.
template<typename T>
bool parse_item(Optional<T>& item,
  PyObject* args,
  Py_ssize_t& n,
  Py_ssize_t len,
  bool allowFlat)
{
  if (n == len){
    // No arguments left, this is OK since this is an optional
    return true;
  }

  if (is_none(args, n, len)){
    n += 1;
    return true;
  }

  typename type2type<T>::type temp;
  if (parse_item(temp, args, n, len, allowFlat)){
    item.Set(temp);
    return true;
  }

  // Unexpected type?
  return false;
}

PyObject* build_result(const Angle&);
PyObject* build_result(const AngleSpan&);
PyObject* build_result(bool);
PyObject* build_result(const Bitmap&);
PyObject* build_result(const BoundObject<Object>&);
PyObject* build_result(const Calibration&);
PyObject* build_result(const Bound<Canvas>&);
PyObject* build_result(const CanvasGrid&);
PyObject* build_result(const Color&);
PyObject* build_result(const ColorStop&);
PyObject* build_result(coord);
PyObject* build_result(const Delay&);
PyObject* build_result(const DirPath&);
PyObject* build_result(const FilePath&);
PyObject* build_result(const Frame&);
PyObject* build_result(const Index&);
PyObject* build_result(int);
PyObject* build_result(const IntPoint&);
PyObject* build_result(const IntRect&);
PyObject* build_result(const IntSize&);
PyObject* build_result(const LineSegment&);
PyObject* build_result(const Object*);
PyObject* build_result(const Paint&);
PyObject* build_result(const PathPt&);
PyObject* build_result(const Point&);
PyObject* build_result(PyObject*);
PyObject* build_result(const Radii&);
PyObject* build_result(const Rect&);
PyObject* build_result(const Settings&);
PyObject* build_result(const std::string&);
PyObject* build_result(const TextLine&);
PyObject* build_result(const Tri&);
PyObject* build_result(const utf8_string&);

template<typename T>
PyObject* build_result(const Optional<T>& opt){
  if (!opt.IsSet()){
    return Py_BuildValue("");
  }
  return build_result(opt.Get());
}

template<typename T1, typename T2>
PyObject* build_result(const std::pair<T1, T2>& pair){
  PyObject* first = build_result(pair.first);
  PyObject* second = build_result(pair.second);
  return PyTuple_Pack(2, first, second);
}

template<typename T>
PyObject* build_result(const std::vector<T>& v){
  PyObject* py_list = PyList_New(resigned(v.size()));
  for (size_t i = 0; i!= v.size(); i++){
    PyList_SetItem(py_list, resigned(i), build_result(v[i]));
  }
  return py_list;
}

template<typename K, typename V>
PyObject* build_result(const std::map<K, V>& m){
  PyObject* py_dict = PyDict_New();

  for (const auto& kv : m){
    // Fixme: Check return values
    PyObject* key = build_result(kv.first);
    PyObject* value = build_result(kv.second);
    int ok = PyDict_SetItem(py_dict, key, value);
    if (ok == -1){
      return nullptr;
    }
  }
  return py_dict;
}

// Default-parse_flat which merely static_asserts
template<typename T>
bool parse_flat(T, PyObject*, Py_ssize_t&, Py_ssize_t){
  static_assert(TypeDependentFalse<T>::value,
    "parse_flat not overloaded for type.");
  return false;
}

template<typename T>
bool parse_flat(Optional<T>& opt, PyObject* args, Py_ssize_t& n, Py_ssize_t len){
  // Fixme: n, length?
  if (args == Py_None){
    return true;
  }
  T obj;
  if (parse_flat(obj, args, n, len)){
    opt.Set(obj);
    return true;
  }
  return false;
}

// Default-build_result which merely static_asserts
template<typename T>
PyObject* build_result(const T&){
  static_assert(TypeDependentFalse<T>::value,
    "build_result not overloaded for type.");
  return nullptr;
}

} // namespace

#endif
