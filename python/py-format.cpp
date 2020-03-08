// -*- coding: us-ascii-unix -*-
// Copyright 2012 Lukas Kemmer
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

#include "app/canvas.hh"
#include "text/formatting.hh"
#include "util/image-props.hh"
#include "python/py-include.hh"
#include "python/py-canvas.hh"
#include "python/py-exception-types.hh"
#include "python/py-format.hh"
#include "python/py-func-context.hh"
#include "python/py-image-props.hh"
#include "python/py-util.hh"

namespace faint{

PyFileFormat::PyFileFormat(const load_callback_t& loader,
  const save_callback_t& saver,
  const label_t& label,
  const FileExtension& ext,
  PyFuncContext& ctx)
  : Format(ext,
      label,
      can_save(saver.Get() != nullptr),
      can_load(loader.Get() != nullptr)),
    m_ctx(ctx)
{
  if (loader.Get() != nullptr){
    m_callLoad = new_ref(loader.Get());
  }
  if (saver.Get() != nullptr){
    m_callSave = new_ref(saver.Get());
  }
}

static utf8_string normal_save_error_prefix(const Format& /*format*/){
  return utf8_string("Save failed\n\n");
}

static utf8_string internal_save_error_prefix(const Format& format){
  return utf8_string("Save Failed (Internal Error)\n\n") +
    utf8_string("An error occurred in the save function for ") +
    format.GetLabel() +
    utf8_string("\n\n");
}

static utf8_string internal_load_error_prefix(const Format& format){
  return utf8_string("An Internal Error occured in the load function for ") +
    format.GetLabel() + utf8_string("\n\n");
}

SaveResult save_error_from_exception(const Format& format){
  if (py_save_error_occurred()){
    // A regular save failure was signalled from the Python code
    // (ifaint.SaveError)
    return SaveResult::SaveFailed(normal_save_error_prefix(format) +
      py_error_string());
  }

  // Some other exception happened, this is an internal error,
  // Add stack trace etc.
  FaintPyExc error = py_error_info();
  return SaveResult::SaveFailed(internal_save_error_prefix(format) +
    error.type + "\n\n" +
    error.message + "\n" + no_sep(error.stackTrace));
}

SaveResult PyFileFormat::Save(const FilePath& filePath, Canvas& canvas){
  assert(m_callSave != nullptr);

  scoped_ref py_canvas(pythoned(canvas, m_ctx));
  scoped_ref filePathUnicode(build_unicode(filePath.Str()));
  scoped_ref argList(Py_BuildValue("OO", filePathUnicode.get(), py_canvas.get()));

  scoped_ref result(PyObject_Call(m_callSave.get(), argList.get(), nullptr));
  if (result == nullptr){
    return save_error_from_exception(*this);
  }
  return SaveResult::SaveSuccessful();
}

utf8_string load_error_string_from_exception(const Format& format){
  if (py_load_error_occurred()){
    // A regular load failure was signalled from the Python code
    // (ifaint.LoadError)
    return py_error_string();
  }

  // Some other exception happened, this is an internal error
  // Add stack trace etc.
  FaintPyExc error = py_error_info();
  return internal_load_error_prefix(format) +
    endline_sep(error.type, error.message) + "\n\n" +
    no_sep(error.stackTrace);
}

void PyFileFormat::Load(const FilePath& filePath, ImageProps& props){
  if (m_callLoad == nullptr){
    props.SetError(space_sep(GetLabel(), "does not support loading."));
    return;
  }

  auto py_props = pythoned(props);
  static_assert(managed(py_props));

  scoped_ref filePathUnicode(build_unicode(filePath.Str()));
  scoped_ref argList(Py_BuildValue("OO", filePathUnicode.get(),
    py_props.get()));
  scoped_ref result(PyObject_Call(m_callLoad.get(), argList.get(), nullptr));
  if (result == nullptr){
    props.SetError(load_error_string_from_exception(*this));
  }

  py_props->alive = false;
}

} // namespace
