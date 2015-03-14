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

#ifndef FAINT_ART_CONTAINER_HH
#define FAINT_ART_CONTAINER_HH
#include <map>
#include "wx/bitmap.h"
#include "wx/cursor.h"
#include "app/resource-id.hh"

namespace faint{

class DirPath;

class ArtContainer{
  // Handles bitmap loading and storage for application art (e.g.
  // icons, buttons). Supports setting a root path for interpreting
  // relative paths.
public:
  ArtContainer();
  wxBitmap Get(Icon id) const;
  const wxCursor& Get(Cursor id) const;
  void Load(const wxString& filename, Cursor id);
  void Load(const wxString& filename, Icon id);
  void Add(const wxCursor&, Cursor id);
  void SetRoot(const DirPath&);

  ArtContainer(const ArtContainer&) = delete;
  ArtContainer& operator=(const ArtContainer&) = delete;
private:
  std::map<Cursor, wxCursor> m_cursors;
  std::map<Icon, wxBitmap> m_icons;
  wxString m_rootPath;
};

} // namespace

#endif
