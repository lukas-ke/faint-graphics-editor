#include "tests/unit-tests/gen/defines.hh"

#if defined(TEST_PLATFORM_LINUX)
// Linux test runner:
// On Linux, I need to initialize a wxApp, or I get
// gtk errors when running e.g. test-faint-dc and test-text-expression.
//
// wxAppConsole was not enough to fix this.
#include "wx/wx.h"
#include "wx/app.h"
#include "wx/cmdline.h"
#include "wx/image.h"

// Defined in generated code.
int run_tests(int argc, char** argv);

static const wxCmdLineEntryDesc g_cmdLineDesc[] = {
  {wxCMD_LINE_SWITCH, "", "silent",
   "",
   wxCMD_LINE_VAL_STRING,
   wxCMD_LINE_PARAM_OPTIONAL},

  {wxCMD_LINE_NONE, "", "", "",
   wxCMD_LINE_VAL_NONE, wxCMD_LINE_PARAM_OPTIONAL} // Sentinel
};

class TestApp : public wxApp{
public:
  int MainLoop() override{
    return run_tests(this->argc, this->argv);
  }

  void OnInitCmdLine(wxCmdLineParser& parser) override{
    parser.SetDesc(g_cmdLineDesc);
  }

  bool OnCmdLineParsed(wxCmdLineParser&) override{
    return true;
  }

  bool OnInit() override{
    if (!wxApp::OnInit()){
      return false;
    }
    m_frame = new wxFrame(nullptr, wxID_ANY, "Dummy");
    // Need to set, or MainLoop isn't called.
    SetTopWindow(m_frame);
    wxInitAllImageHandlers();
    return true;
  }
private:
  wxFrame* m_frame;
};

wxIMPLEMENT_APP(TestApp);
#elif defined(TEST_PLATFORM_WINDOWS)
// Windows test-runner:
// On Windows, getting a wxAppConsole to use the host console
// is a bit of work, as I must use subsystem=Windows.
// I don't seem to need to initialize a wxApp there, so just call
// run_tests.
#include "wx/image.h"

int run_tests(int, char**);

int main(int argc, char** argv){
  wxInitAllImageHandlers();
  return run_tests(argc, argv);
}

#else
#error Expected a platform
#endif
