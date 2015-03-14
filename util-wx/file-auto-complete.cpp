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

#include "wx/dir.h"
#include "wx/filename.h"
#include "util-wx/file-auto-complete.hh"

namespace faint{

static std::vector<wxString> list_files(const wxDir& dir, const wxString& match){
  std::vector<wxString> files;
  wxString filename;
  if (dir.GetFirst(&filename, match + "*")){
    files.push_back(filename);
    while (dir.GetNext(&filename)){
      files.push_back(filename);
    }
  }
  return files;
}

FileAutoComplete::FileAutoComplete()
  : m_index(0)
{}

wxString FileAutoComplete::Complete(const wxString& root){
  Forget();

  wxString path, file, ext;
  wxFileName::SplitPath(wxString(root), &path, &file, &ext);
  if (wxDirExists(path)){
    m_root = path;
    m_root.Replace("\\", "/");
    if (m_root[m_root.size() - 1] != '/'){
      m_root += '/';
    }
    m_matches = list_files(wxDir(path), file);
  }
  return m_matches.empty() ?
    root :
    GetItem(0);
}

void FileAutoComplete::Forget(){
  m_index = 0;
  m_matches.clear();
}

wxString FileAutoComplete::GetItem(size_t item) const{
  if (m_matches.empty()){
    return m_root;
  }
  return m_root + m_matches[item % m_matches.size()];
}

bool FileAutoComplete::Has() const{
  return !m_matches.empty();
}

wxString FileAutoComplete::Next(){
  return GetItem(++m_index);
}

wxString FileAutoComplete::Previous(){
  return GetItem(--m_index);
}

} // namespace
