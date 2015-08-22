#include <cassert>
#include <stdexcept>
#include <sstream>

// Stubs required for linking the GUI test

namespace faint{

} // namespace

namespace test{
// Some things I need to be able to link the test utils.
// FIXME: Try getting rid of at least TEST_FAILED and TEST_OUT
bool TEST_FAILED = false;
std::stringstream TEST_OUT;
}
std::string get_test_name(){
  return "GuiTest";
}
