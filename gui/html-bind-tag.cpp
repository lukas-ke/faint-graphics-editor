// -*- coding: us-ascii-unix -*-
// Copyright 2013 Lukas Kemmer
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

#include "wx/dcscreen.h"
#include "wx/html/m_templ.h"
#include "wx/html/htmlcell.h"
#include "python/py-key-press.hh"
#include "util-wx/convert-wx.hh"
#include "util-wx/file-path-util.hh"

namespace faint{

utf8_string get_bound_key(const utf8_string& function){
  std::vector<BindInfo> binds = list_binds();
  for (size_t i = 0; i != binds.size(); i++){
    if (binds[i].function == function){
      const BindInfo& b(binds[i]);
      return b.key.Name();
    }
  }
  return "";
}

// This adds the tag <BIND> to wxHtml.
// When parsed, looks up if the content in <bind> matches
// a Python-function bound to a key, e.g. <bind>zoom_in</bind>,
// and if so, lists the key.
TAG_HANDLER_BEGIN(BIND, "BIND")
  TAG_HANDLER_CONSTR(BIND) { }

  TAG_HANDLER_PROC(tag){
    wxString str = m_WParser->GetInnerSource(tag);
    utf8_string key = get_bound_key(to_faint(str));
    if (!key.empty()){
      m_WParser->GetContainer()->InsertCell(new wxHtmlWordCell("(Key: "+
        to_wx(key)+ ")", wxScreenDC()));
    }
    return true; // Eat inner content
  }
TAG_HANDLER_END(BIND)

// The <BINDLIST> tag
TAG_HANDLER_BEGIN(BINDLIST, "BINDLIST")
  TAG_HANDLER_CONSTR(BINDLIST){}

  TAG_HANDLER_PROC(tag){
    std::vector<BindInfo> binds = list_binds();
    m_WParser->GetInnerSource(tag); // Just to silence unused parameter warning for "tag".
    // Create a table of binds
    for (size_t i = 0; i != binds.size(); i++){
      const BindInfo& b(binds[i]);

      // A container for the key-text
      wxHtmlContainerCell* cell = m_WParser->OpenContainer();
      // Use 20% of the width for the key
      cell->SetWidthFloat(20, wxHTML_UNITS_PERCENT);
      cell->InsertCell(new wxHtmlWordCell(to_wx(b.key.Name()), wxScreenDC()));
      m_WParser->CloseContainer();

      // A container for the function name
      cell = m_WParser->OpenContainer();
      // Use 80% of the width for the function name
      cell->SetWidthFloat(80, wxHTML_UNITS_PERCENT);
      cell->InsertCell(new wxHtmlWordCell(to_wx(b.function), wxScreenDC()));
      m_WParser->CloseContainer();
    }

    return true; // Eat inner content
  }
TAG_HANDLER_END(BINDLIST)

// The <FAINTCONFIGPATH> tag
TAG_HANDLER_BEGIN(CONFIG_PATH_TAG, "FAINTCONFIGPATH")
  TAG_HANDLER_CONSTR(CONFIG_PATH_TAG){}

  TAG_HANDLER_PROC(tag){
    wxString iniPath = to_wx(get_user_config_file_path().Str());
    m_WParser->GetContainer()->InsertCell(new wxHtmlWordCell(iniPath,
      wxScreenDC()));
    m_WParser->GetInnerSource(tag); // Just to silence unused parameter warning for "tag".
    return true; // Eat inner content
  }
TAG_HANDLER_END(CONFIG_PATH_TAG)


// Add the custom Faint tags as a tag-module
TAGS_MODULE_BEGIN(HFAINT)
TAGS_MODULE_ADD(BIND)
TAGS_MODULE_ADD(BINDLIST)
TAGS_MODULE_ADD(CONFIG_PATH_TAG)
TAGS_MODULE_END(HFAINT)

} // namespace faint
