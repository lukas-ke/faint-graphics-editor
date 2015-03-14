// -*- coding: us-ascii-unix -*-
// Copyright 2014 Lukas Kemmer
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

#include "tablet/msw/tablet-error.hh"
#include "tablet/msw/tablet-error-message.hh"
#include "util-wx/gui-util.hh"

namespace faint{namespace tablet{

void show_tablet_error_message(wxWindow* parent, InitResult error){
  switch(error){
    case tablet::InitResult::OK:
      assert(false);
      break;
    case tablet::InitResult::DLL_NOT_FOUND:
      // No tablet DLL available. This is not an error.
      break;
    case tablet::InitResult::ERROR_LOGCONTEXT_SIZE:
      show_error(parent, Title("Tablet error"),
        "Wintab could not be initialized (ERROR_LOGCONTEXT_SIZE).\n\n"
        "Pen tablet-functionality will not be available.");
      break;
    case tablet::InitResult::OTHER_ERROR:
      show_error(parent, Title("Tablet error"),
        "Wintab could not be initialized (OTHER_ERROR).\n\n"
        "Pen tablet-functionality will not be available.");
      break;
    case tablet::InitResult::WTOPENA_FAILED:
      show_error(parent, Title("Tablet error"),
        "Wintab could not be initialized (Call to WTOpenA failed).\n\n"
        "Pen tablet-functionality will not be available.");
      break;
    default:
      assert(false);
  }
}

}} // namespace
