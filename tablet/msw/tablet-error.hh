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

#ifndef FAINT_TABLET_ERROR_HH
#define FAINT_TABLET_ERROR_HH

namespace faint{namespace tablet{

enum class InitResult{
  OK,

  // Wintab32.dll not available
  DLL_NOT_FOUND,

  // Size of WTI_DEFSYSCTX does not match statically compiled struct.
  ERROR_LOGCONTEXT_SIZE,

  // Call to WTOpenA failed
  WTOPENA_FAILED,

  OTHER_ERROR
};

}} // namespace

#endif
