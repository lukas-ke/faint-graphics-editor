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

#include <sstream>
#include "wx/filename.h"
#include "wx/ipc.h"
#include "wx/snglinst.h"
#include "app/get-app-context.hh"
#include "app/one-instance.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"

namespace faint{

const char* LOCALHOST = "localhost";
static const wxString FAINT_TOPIC_FILES("faint-files");
static const wxString FAINT_TOPIC_RAISE("faint-raise");
static const wxString FILE_PREFIX = "file:";
static const wxString MESSAGE_COMPLETE = "done";

static bool is_file_message(const wxString& message){
  if (message.size() < FILE_PREFIX.size()){
    return false;
  }
  return message.substr(0, FILE_PREFIX.size()) == FILE_PREFIX;
}

static bool is_completion_message(const wxString& message){
  return message == MESSAGE_COMPLETE;
}

static wxFileName extract_file_path(const wxString& message){
  assert(is_file_message(message));
  wxString path(message.substr(FILE_PREFIX.size()));
  return wxFileName(path);
}

class FaintConnection : public wxConnection {
public:
  bool OnExec(const wxString& /*topic*/, const wxString& message) override{
    if (is_file_message(message)){
      // A file message from a different instance
      wxFileName pathWx(extract_file_path(message));
      if (pathWx.IsAbsolute() && !pathWx.IsDir()){
        m_files.push_back(FilePath::FromAbsoluteWx(pathWx));
      }
      else{
        // Do nothing on bad file name
      }
    }
    else if (is_completion_message(message)){
      // QueueLoadFiles returns immediately, this avoids the sending
      // application from timing out, and showing a wxWidgets error
      // message.  (The sender doesn't care if the files load or not -
      // the other Faint instance should deal with the error
      // reporting)
      get_app_context().QueueLoad(m_files);
      m_files.clear();
    }
    return true;
  }
private:
  FileList m_files;
};

class FaintClient : public wxClient {
public:
  wxConnectionBase* OnMakeConnection() override{
    return new FaintConnection;
  }
};

class FaintServer : public wxServer {
public:
  wxConnectionBase* OnAcceptConnection(const wxString& topic) override{
    if (topic == FAINT_TOPIC_FILES){
      return new FaintConnection;
    }
    else if (topic == FAINT_TOPIC_RAISE){
      get_app_context().RaiseWindow();
      return new FaintConnection;
    }
    else {
      return nullptr;
    }
  }
};

static void show_start_server_error(const wxString& port){
    std::stringstream ss;
    ss << "Failed to create an IPC-service" << std::endl << std::endl <<
      "Using a single instance of Faint, and passing file names to the "
      "running instance will not work." << std::endl <<
      "Port: " << port;
    wxLogError(ss.str().c_str());
}

static std::unique_ptr<FaintServer> start_faint_server(const std::string& port){
  auto server = std::make_unique<FaintServer>();
  bool ok = server->Create(port);
  if (!ok){
    show_start_server_error(port);
    return nullptr;
  }
  return server;
}

static bool send_paths_to_server(const FileList& paths, const std::string& port){
  FaintClient client;
  std::unique_ptr<wxConnectionBase> connection(
    client.MakeConnection(LOCALHOST, port, FAINT_TOPIC_FILES));

  if (connection == nullptr){
    // Failed connecting to the old instance
    return false;
  }

  // Pass all the files to the old instance
  for (const FilePath& filePath : paths){
    connection->Execute(FILE_PREFIX + to_wx(filePath.Str()));
  }

  connection->Execute(MESSAGE_COMPLETE);
  connection->Disconnect();
  return true;
}

static bool raise_existing_window(const std::string& port){
  FaintClient client;
  std::unique_ptr<wxConnectionBase> connection(client.MakeConnection(LOCALHOST,
    port, FAINT_TOPIC_RAISE));

  if (connection == nullptr){
    // Connecting to the old instance failed.
    return false;
  }

  connection->Disconnect();
  return true;
}

using allow_start = Distinct<bool, FaintInstance, 10>;

class FaintInstanceImpl : public FaintInstance{
public:
  FaintInstanceImpl(std::unique_ptr<wxServer> server,
    std::unique_ptr<wxSingleInstanceChecker> singleInst,
    const allow_start& allowStart)
    : m_allowStart(allowStart.Get()),
      m_server(std::move(server)),
      m_singleInstance(std::move(singleInst))
  {}

  bool AllowStart() const override{
    return m_allowStart;
  }

private:
  bool m_allowStart;
  std::unique_ptr<wxServer> m_server;
  std::unique_ptr<wxSingleInstanceChecker> m_singleInstance;
};

static std::unique_ptr<wxSingleInstanceChecker> get_instance_checker(){
  const wxString name = wxString::Format("Faint-%s", wxGetUserId().c_str());
  return std::make_unique<wxSingleInstanceChecker>(name);
}

std::unique_ptr<FaintInstance> create_faint_instance(const FileList& cmdLineFiles,
  const allow_server& allowServer,
  const force_start& forceStart,
  const std::string& port)
{
  // Create the first instance, if possible
  std::unique_ptr<wxSingleInstanceChecker> singleInst(get_instance_checker());

  if (!singleInst->IsAnotherRunning()){
    std::unique_ptr<wxServer> server;
    if (allowServer.Get()){
      server = start_faint_server(port);
    }
    else{
      singleInst.reset(nullptr);
    }
    return std::make_unique<FaintInstanceImpl>(std::move(server),
      std::move(singleInst), allow_start(true));
  }

  if (forceStart.Get()){
    return std::make_unique<FaintInstanceImpl>(nullptr, nullptr,
      allow_start(true));
  }

  if (cmdLineFiles.empty()){
    // Faint is running since previously, so just raise the window.
    if (raise_existing_window(port)){
      return std::make_unique<FaintInstanceImpl>(nullptr, nullptr,
        allow_start(false));
    }
    // Failed raising old Faint - something is amiss, allow a new
    // instance.
    return std::make_unique<FaintInstanceImpl>(nullptr, nullptr,
      allow_start(true));
  }

  // Faint is running since previously and should be passed the
  // filenames from this instance.
  if (send_paths_to_server(cmdLineFiles, port)){
    // The files were sent to the running instance. Prevent
    // this instance from starting.
    return std::make_unique<FaintInstanceImpl>(nullptr, nullptr,
      allow_start(false));
  }

  // Failed sending paths to old instance - something is amiss,
  // allow a new instance.
  return std::make_unique<FaintInstanceImpl>(nullptr, nullptr, allow_start(true));
}

} // namespace
