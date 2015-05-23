#include <cassert>
#include <stdexcept>

// Stubs required for linking the tests.
//
// Fixme: Preferably separate the tests into those needing
// (at least link-time) application-contexts and those that don't.

namespace faint{
class AppContext;
class Art;

AppContext& get_app_context(){
  assert(false);
  throw std::logic_error("Stub get_app_context called");
}

const Art& get_art(){
  assert(false);
  throw std::logic_error("Stub get_art called");
}

class PythonContext;
PythonContext& get_python_context(){
  assert(false);
  throw std::logic_error("Stup get_python_context called");
}

} // namespace
