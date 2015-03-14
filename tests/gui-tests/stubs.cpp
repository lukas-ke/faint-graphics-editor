#include <cassert>
#include <stdexcept>
#include <sstream>

// Stubs required for linking the GUI test

namespace faint{
class AppContext;
class ArtContainer;

AppContext& get_app_context(){
  assert(false);
  throw std::logic_error("Stub get_app_context called");
}

class PythonContext;
PythonContext& get_python_context(){
  assert(false);
  throw std::logic_error("Stup get_python_context called");
}

} // namespace

// Some things I need to be able to link the test utils.
// FIXME: Try getting rid of at least TEST_FAILED and TEST_OUT
bool TEST_FAILED = false;
std::stringstream TEST_OUT;
std::string get_test_name(){
  return "GuiTest";
}
