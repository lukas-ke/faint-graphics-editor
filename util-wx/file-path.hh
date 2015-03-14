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

#ifndef FAINT_FILE_PATH_HH
#define FAINT_FILE_PATH_HH
#include <string>
#include <vector>
#include "util/template-fwd.hh"

class wxFileName;
class wxString;

namespace faint{

class utf8_string;
class ExtensionImpl;

class FileExtension{
public:
  explicit FileExtension(const wxString&);
  explicit FileExtension(const char*);
  FileExtension(const FileExtension&);
  FileExtension(FileExtension&&);
  FileExtension& operator=(const FileExtension&);
  ~FileExtension();
  utf8_string Str() const;

  bool operator==(const FileExtension&) const;
private:
  ExtensionImpl* m_impl;
};

class DirPathImpl;
class FilePath;

class DirPath{
public:
  explicit DirPath(const wxString&);
  DirPath(const DirPath&);
  ~DirPath();

  DirPath SubDir(const wxString&) const;
  DirPath SubDir(const char*) const;
  FilePath File(const utf8_string&) const;
  utf8_string Str() const;
  DirPath& operator=(const DirPath&) = delete;

  DirPathImpl* m_impl;
};

class FileNameImpl;

class FileName{
public:
  explicit FileName(const wxString&);
  explicit FileName(const utf8_string&);
  explicit FileName(const char*);
  FileName(const FileName&);
  ~FileName();
  utf8_string Str() const;
private:
  FileNameImpl* m_impl;
};

class PathImpl;
class FilePath{
  // This class chokes on non-absolute paths and tries to choke on
  // non-file paths as well, as an attempt to keep all stored file
  // paths in Faint absolute.

public:
  // Creates a FilePath. Asserts if the passed in path is not absolute
  // or does not refer to a file.
  static FilePath FromAbsoluteWx(const wxFileName&);
  static FilePath FromAbsolute(const utf8_string&);
  FilePath(const FilePath&);
  ~FilePath();
  FilePath& operator=(const FilePath& other);
  DirPath StripFileName() const;
  FileExtension Extension() const;
  FileName StripPath() const;
  utf8_string Str() const;

  const PathImpl* GetImpl() const;
private:
  explicit FilePath(PathImpl*);
  PathImpl* m_impl;
};

class FileListIter{
public:
  FileListIter& operator++();
  bool operator!=(const FileListIter&);
  const FilePath& operator*() const;
private:
  friend class FileList;
  using iter_type = std::vector<FilePath*>::const_iterator;
  explicit FileListIter(const iter_type&);
  iter_type m_it;
};

class FileList{
  // Simple vector-like container for FilePath:s, since I don't want
  // to add a default constructor to FilePath
public:
  FileList();
  FileList(const FileList&);
  ~FileList();
  const FilePath& back() const;
  FileListIter begin() const;
  void clear();
  const FilePath& operator[](size_t) const;
  bool empty() const;
  FileListIter end() const;
  void push_back(const FilePath&);
  size_t size() const;

  FileList& operator=(const FileList&) = delete;
private:
  std::vector<FilePath*> m_files;
};

bool exists(const FilePath&);
bool exists(const DirPath&);
bool make_dir(const DirPath&);
bool is_absolute_path(const utf8_string&);
bool is_file_path(const utf8_string&);

#ifdef FAINT_MSW
// In Visual Studios iostream implementation, char* filenames are
// expected to be ANSI, and wchar_t* are expected to be UTF-16.
std::wstring iostream_friendly(const FilePath&);
#else
// Assume other platforms treat char* as UTF8
std::string iostream_friendly(const FilePath&);
#endif

Optional<FilePath> make_absolute_file_path(const utf8_string&);

} // namespace

#endif
