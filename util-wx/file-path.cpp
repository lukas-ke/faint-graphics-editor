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
#include "text/utf8-string.hh"
#include "util/optional.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"

namespace faint{

class ExtensionImpl{
 public:
  ExtensionImpl(const utf8_string& extension)
    : extension(extension)
  {}

  utf8_string extension;
};

FileExtension::FileExtension(const wxString& s)
  : m_impl(new ExtensionImpl(to_faint(s)))
{}

FileExtension::FileExtension(const char* s)
  : m_impl(new ExtensionImpl(utf8_string(s)))
{}

FileExtension::FileExtension(const FileExtension& other)
  : m_impl(new ExtensionImpl(other.m_impl->extension))
{}

FileExtension::FileExtension(FileExtension&& other)
  : m_impl(nullptr)
{
  std::swap(other.m_impl, m_impl);
}

FileExtension::~FileExtension(){
  delete m_impl;
}

FileExtension& FileExtension::operator=(const FileExtension& other){
  assert(m_impl != nullptr);
  m_impl->extension = other.m_impl->extension;
  return *this;
}

utf8_string FileExtension::Str() const{
  assert(m_impl != nullptr);
  return m_impl->extension;
}

bool FileExtension::operator==(const FileExtension& other) const{
  return to_wx(m_impl->extension).Lower() ==
    to_wx(other.m_impl->extension).Lower();
}

class DirPathImpl{
public:
  DirPathImpl(const wxString& dirPath)
    : dirPath(dirPath)
  {}

  wxString dirPath;
};

DirPath::DirPath(const DirPath& other)
  : m_impl(new DirPathImpl(other.m_impl->dirPath))
{}

DirPath::DirPath(const wxString& dirPath) : m_impl(new DirPathImpl(dirPath))
{}

DirPath::~DirPath(){
  delete m_impl;
}

utf8_string DirPath::Str() const{
  return to_faint(m_impl->dirPath);
}

FilePath DirPath::File(const utf8_string& filename) const{
  return FilePath::FromAbsoluteWx(m_impl->dirPath +
    wxFileName::GetPathSeparator() + to_wx(filename));
}

DirPath DirPath::SubDir(const wxString& dir) const{
  assert(wxFileName(dir).IsRelative());
  return DirPath(m_impl->dirPath + wxFileName::GetPathSeparator() +
    dir);
}

DirPath DirPath::SubDir(const char* dir) const{
  assert(wxFileName(dir).IsRelative());
  return DirPath(m_impl->dirPath + wxFileName::GetPathSeparator() +
    wxString(dir));
}

class FileNameImpl{
public:
  FileNameImpl(const wxString& fileName) :
    fileName(fileName)
  {}
  wxString fileName;
};

FileName::FileName(const wxString& fileName)
  : m_impl(new FileNameImpl(fileName))
{}

FileName::FileName(const utf8_string& str)
  : m_impl(new FileNameImpl(to_wx(str)))
{}

FileName::FileName(const char* str)
  : m_impl(new FileNameImpl(str))
{}

FileName::FileName(const FileName& other)
  : m_impl(new FileNameImpl(other.m_impl->fileName))
{}

FileName::~FileName(){
  delete m_impl;
}

utf8_string FileName::Str() const{
  return to_faint(m_impl->fileName);
}

class PathImpl{
public:
  PathImpl(const wxFileName& path)
    : path(path)
  {}
  wxFileName path;
};

FilePath::~FilePath(){
  delete m_impl;
}

FilePath FilePath::FromAbsoluteWx(const wxFileName& wxFn){
  assert(wxFn.IsAbsolute());
  assert(!wxFn.IsDir());
  return FilePath(new PathImpl(wxFn));
}

FilePath FilePath::FromAbsolute(const utf8_string& filename){
  wxFileName wxFn(to_wx(filename));
  assert(wxFn.IsAbsolute());
  assert(!wxFn.IsDir());
  return FilePath(new PathImpl(wxFn));
}

FilePath::FilePath(PathImpl* impl){
  m_impl = impl;
}

FilePath::FilePath(const FilePath& other){
  m_impl = new PathImpl(*other.m_impl);
}

FileExtension FilePath::Extension() const{
  return FileExtension(m_impl->path.GetExt());
}

const PathImpl* FilePath::GetImpl() const{
  return m_impl;
}

utf8_string FilePath::Str() const{
  return to_faint(m_impl->path.GetFullPath());
}

FileName FilePath::StripPath() const{
  wxString name;
  wxString ext;
  bool hasExt;
  wxFileName::SplitPath(m_impl->path.GetFullPath(),
    0, // Volume
    0, // Path
    &name,
    &ext,
    &hasExt);

  if (!hasExt){
    return FileName(name);
  }
  return FileName(name + wxString(".") + ext);
}

FilePath& FilePath::operator=(const FilePath& other){
  if (this == &other){
    return *this;
  }

  if (other.m_impl == nullptr){
    m_impl = nullptr;
  }
  else{
    delete m_impl;
    m_impl = new PathImpl(*other.m_impl);

  }
  return *this;
}

DirPath FilePath::StripFileName() const{
  return DirPath(m_impl->path.GetPath());
}

FileListIter::FileListIter(const iter_type& it)
  : m_it(it)
{}

FileListIter& FileListIter::operator++(){
  m_it++;
  return *this;
}

bool FileListIter::operator!=(const FileListIter& other){
  return m_it != other.m_it;
}

const FilePath& FileListIter::operator*() const{
  return **m_it;
}

FileList::FileList(){
}

FileList::FileList(const FileList& other){
  for (const FilePath* path : other.m_files){
    push_back(*path);
  }
}

FileList::~FileList(){
  clear();
}

  const FilePath& FileList::back() const{
  assert(!m_files.empty());
  return *m_files.back();
}

FileListIter FileList::begin() const{
  return FileListIter(m_files.begin());
}

void FileList::clear(){
  for (FilePath* path : m_files){
    delete path;
  }
  m_files.clear();
}

bool FileList::empty() const{
  return m_files.empty();
}

FileListIter FileList::end() const{
  return FileListIter(m_files.end());
}

const FilePath& FileList::operator[](size_t i) const{
  assert(i < m_files.size());
  return *m_files[i];
}

void FileList::push_back(const FilePath& path){
  m_files.push_back(new FilePath(path));
}

size_t FileList::size() const{
  return m_files.size();
}

bool exists(const FilePath& path){
  return path.GetImpl()->path.FileExists();
}

bool exists(const DirPath& path){
  return wxDir::Exists(to_wx(path.Str()));
}

bool make_dir(const DirPath& path){
  return wxDir::Make(to_wx(path.Str()));
}

bool is_absolute_path(const utf8_string& path){
  return wxFileName(to_wx(path)).IsAbsolute();
}

bool is_file_path(const utf8_string& path){
  return !wxFileName(to_wx(path)).IsDir();
}

#ifdef FAINT_MSW
std::wstring iostream_friendly(const FilePath& path){
  return iostream_friendly(to_wx(path.Str()));
}
#else
  std::string iostream_friendly(const FilePath& path){
    return path.Str().str();
}
#endif

Optional<FilePath> make_absolute_file_path(const utf8_string& path){
  wxFileName pathWX(to_wx(path));
  if (!pathWX.IsOk()){
    return no_option();
  }
  pathWX = absoluted(pathWX);
  return option(FilePath::FromAbsoluteWx(pathWX));
}

} // namespace
