#include <cassert>
#include <stdexcept>

// Stubs required for linking the tests.
//
// Fixme: Preferably separate the tests into those needing
// (at least link-time) application-contexts and those that don't.

namespace faint{
class AppContext;
class ArtContainer;

AppContext& get_app_context(){
  assert(false);
  throw std::logic_error("Stub get_app_context called");
}

const ArtContainer& get_art_container(){
  assert(false);
  throw std::logic_error("Stub get_art_container called");
}

class PythonContext;
PythonContext& get_python_context(){
  assert(false);
  throw std::logic_error("Stup get_python_context called");
}

} // namespace
