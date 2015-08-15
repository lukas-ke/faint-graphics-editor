// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#include "text/slice.hh"
#include "text/slice-generic.hh"

namespace faint{

utf8_string slice(const utf8_string& s, int from, int up_to){
  return generic::slice(s, from, up_to);
}

utf8_string slice_from(const utf8_string& s, int first){
  return generic::slice_from(s, first);
}

utf8_string slice_up_to(const utf8_string& s, int up_to){
  return generic::slice_up_to(s, up_to);
}

std::string slice(const char* s, int first, int up_to){
  return generic::slice(std::string(s), first, up_to);
}

std::string slice_from(const char* s, int first){
  return generic::slice_from(std::string(s), first);
}

std::string slice_up_to(const char* s, int up_to){
  return generic::slice_up_to(std::string(s), up_to);
}

std::string slice(const std::string& s, int from, int up_to){
  return generic::slice(s, from, up_to);
}

std::string slice_from(const std::string& s, int first){
  return generic::slice_from(s, first);
}

std::string slice_up_to(const std::string& s, int up_to){
  return generic::slice_up_to(s, up_to);
}

} // namespace
