#ifndef FAINT_PY_FUNC_CONTEXT_HH
#define FAINT_PY_FUNC_CONTEXT_HH
#include "gui/art.hh" // Fixme

namespace faint{

// Fixme: Better name and all, don't mix up with PythonContext.
class PyFuncContext{
public:
  PyFuncContext(Art& art) : art(art){}
  const Art& art;
};

}

#endif
