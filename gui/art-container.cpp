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

#include "wx/filename.h"
#include "wx/msgdlg.h"
#include "wx/bitmap.h"
#include "wx/cursor.h"
#include "bitmap/bitmap.hh"
#include "formats/bmp/file-cur.hh"
#include "gui/art-container.hh"
#include "text/formatting.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path.hh"
#include "util/exception.hh"

namespace faint{

static wxCursor cur_from_bmp(const wxBitmap& bmp, const HotSpot& hotSpot){
  wxImage img(bmp.ConvertToImage());
  img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_X, hotSpot.x);
  img.SetOption(wxIMAGE_OPTION_CUR_HOTSPOT_Y, hotSpot.y);
  img.SetMaskColour(255, 0, 255);
  return wxCursor(img);
}

wxBitmap ArtContainer::Get(Icon iconId) const{
  std::map<Icon, wxBitmap>::const_iterator it = m_icons.find(iconId);
  assert(it != m_icons.end());
  return it->second;
}

const wxCursor& ArtContainer::Get(Cursor cursorId) const{
  std::map<Cursor, wxCursor>::const_iterator it = m_cursors.find(cursorId);
  assert(it != m_cursors.end());
  return it->second;
}

void ArtContainer::Add(const wxCursor& cursor, Cursor cursorId){
  m_cursors[cursorId] = cursor;
}

void ArtContainer::Load(const wxString& filename, Icon iconId){
  wxFileName fn_filename(filename);
  if (fn_filename.IsRelative()){
    fn_filename.MakeAbsolute(m_rootPath);
  }

  if (!fn_filename.FileExists()){
    throw Exception(lbl("Missing icon", to_faint(filename)));
  }

  wxImage image(fn_filename.GetLongPath(), wxBITMAP_TYPE_ANY);
  if (!image.IsOk()){
    throw Exception(lbl("Failed loading icon", to_faint(filename)));
  }

  if (image.HasMask() && !image.HasAlpha()){
    image.InitAlpha();
  }
  m_icons[iconId] = wxBitmap(image);
}

static void throw_unless_single_frame(const cur_vec& cursors,
  const wxString& filename)
{
  if (cursors.size() != 1){
    throw Exception(space_sep("Cursor", to_faint(filename),
      "has", str_uint(cursors.size()), "frames - expected 1."));
  }
}

void ArtContainer::Load(const wxString& filename, Cursor cursorId){
  wxFileName fn_filename(filename);
  if (fn_filename.IsRelative()){
    fn_filename.MakeAbsolute(m_rootPath);
  }
  if (!fn_filename.FileExists()){
    throw Exception(lbl("Missing cursor", to_faint(filename)));
  }
  if (fn_filename.GetExt() != "cur"){
    throw Exception(space_sep("Cursor file-name", bracketed(to_faint(filename)),
      "does not end with .cur"));
  }

  auto result = read_cur(FilePath::FromAbsoluteWx(fn_filename));
  result.Visit(
    [&](const cur_vec& cursors){
      throw_unless_single_frame(cursors, filename);
      const auto& c = cursors.back();
      m_cursors[cursorId] = cur_from_bmp(to_wx_bmp(c.first), c.second);
    },
    [](const utf8_string& err){
      throw Exception(err);
    });
}

void ArtContainer::SetRoot(const DirPath& rootPath){
  m_rootPath = to_wx(rootPath.Str());
}

} // namespace
