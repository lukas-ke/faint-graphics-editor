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

#ifndef FAINT_CONVENIENCE_HH
#define FAINT_CONVENIENCE_HH

namespace faint{

// True if flag set in bits
bool fl(int flag, int bits);

// True if both parameters are false
bool neither(bool, bool);

// Makes the parameter false, returns the old value.
bool then_false(bool&);

} // namespace

#endif
