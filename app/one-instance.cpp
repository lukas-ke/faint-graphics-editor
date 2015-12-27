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
#include "app/app-context.hh"
#include "app/one-instance.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"
#include "util-wx/slice-wx.hh"

#ifdef __WXMSW__
static_assert(wxUSE_DDE_FOR_IPC == 1, "DDE should be used for IPC on windows");
#else
static_assert(wxUSE_DDE_FOR_IPC == 0, "TCP should be used for IPC on non-windows");
#endif

namespace faint{

const char* LOCALHOST = "localhost";
static const wxString FAINT_TOPIC_FILES("faint-files");
static const wxString FAINT_TOPIC_RAISE("faint-raise");
static const wxString FILE_PREFIX = "file:";
static const wxString MESSAGE_COMPLETE = "done";

static bool is_file_message(const wxString& message){
  return slice_up_to(message, FILE_PREFIX.size()) == FILE_PREFIX;
}

static bool is_completion_message(const wxString& message){
  return message == MESSAGE_COMPLETE;
}

static wxFileName extract_file_path(const wxString& message){
  assert(is_file_message(message));
  wxString path(slice_from(message, FILE_PREFIX.size()));
  return wxFileName(path);
}

class FaintConnection : public wxConnection {
public:
  FaintConnection(AppContext& app)
    : m_app(app)
  {}

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
      m_app.QueueLoad(m_files);
      m_files.clear();
    }
    return true;
  }
private:
  AppContext& m_app;
  FileList m_files;
};

class FaintClient : public wxClient {
public:
  wxConnectionBase* OnMakeConnection() override{
    return new wxConnection();
  }
private:
};

class FaintServer : public wxServer {
public:
  FaintServer(AppContext& app)
    : m_app(app)
  {}

  wxConnectionBase* OnAcceptConnection(const wxString& topic) override{
    if (topic == FAINT_TOPIC_FILES){
      return new FaintConnection(m_app);
    }
    else if (topic == FAINT_TOPIC_RAISE){
      m_app.RaiseWindow();
      return new FaintConnection(m_app);
    }
    else{
      return nullptr;
    }
  }
private:
  AppContext& m_app;
};

static void show_start_server_error(const wxString& serviceName){
    std::stringstream ss;
    ss << "Failed to create an IPC-service" << std::endl << std::endl <<
      "Using a single instance of Faint, and passing file names to the "
      "running instance will not work." << std::endl <<
      "Service name: " << serviceName;
    wxLogError(ss.str().c_str());
}

static std::unique_ptr<FaintServer> start_faint_server(AppContext& app,
  const std::string& port)
{
  auto server = std::make_unique<FaintServer>(app);
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
  for (const auto& filePath : paths){
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

class RemoteFaintImpl : public RemoteFaint{
public:
  RemoteFaintImpl(const std::string& serviceName)
    : m_serviceName(serviceName)
  {}

  virtual bool Notify(const FileList& files){
    if (files.empty()){
      // No files specified, just raise the running window.
      return raise_existing_window(m_serviceName);
    }
    else{
      return send_paths_to_server(files, m_serviceName);
    }
  }

  std::string m_serviceName;
};

class FaintInstanceImpl : public FaintInstance{
public:
  FaintInstanceImpl(std::unique_ptr<wxSingleInstanceChecker> singleInst)
    : m_singleInstance(std::move(singleInst))
  {}

  bool IsAnotherRunning() override{
    return m_singleInstance->IsAnotherRunning();
  }

  std::unique_ptr<RemoteFaint> OtherInstance(const std::string& serviceName)
    override
  {
    return std::make_unique<RemoteFaintImpl>(serviceName);
  }

  void StartServer(AppContext& app,
    const std::string& serviceName) override
  {
    assert(m_server == nullptr); // Should only be called once.
    m_server = start_faint_server(app, serviceName);
  }

private:
  std::unique_ptr<wxSingleInstanceChecker> m_singleInstance;
  std::unique_ptr<FaintServer> m_server;
};

static std::unique_ptr<wxSingleInstanceChecker> get_instance_checker(){
  const wxString name = wxString::Format("Faint-%s", wxGetUserId().c_str());
  return std::make_unique<wxSingleInstanceChecker>(name);
}

std::unique_ptr<FaintInstance> create_faint_instance()
{
  return std::make_unique<FaintInstanceImpl>(get_instance_checker());
}

} // namespace
