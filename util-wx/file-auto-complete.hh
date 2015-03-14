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

#ifndef FAINT_FILE_AUTO_COMPLETE_HH
#define FAINT_FILE_AUTO_COMPLETE_HH
#include <vector>
#include "wx/string.h"

namespace faint{

class FileAutoComplete{
  // File-name auto-completion.
public:
  FileAutoComplete();

  // Attempts to auto-complete the specified file path root. Returns
  // the first completion or the root if no completion existed.
  // Further completions can be retrieved with Next().
  wxString Complete(const wxString&);

  // Clears the list of completions.
  void Forget();

  // True if there are completions available (accessed with Next or
  // Previous).
  bool Has() const;

  // Returns the next completion, or the root if no completion exists.
  wxString Next();

  // Returns the previous completion, or the root if no completion
  // exists.
  wxString Previous();
private:
  wxString GetItem(size_t) const;
  wxString m_root;
  size_t m_index;
  std::vector<wxString> m_matches;
};

} // namespace

#endif
