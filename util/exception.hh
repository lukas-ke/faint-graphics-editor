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

#ifndef FAINT_EXCEPTION_HH
#define FAINT_EXCEPTION_HH
#include <exception>
#include "text/utf8-string.hh"

namespace faint{

class Exception : public std::exception{
public:
  explicit Exception(utf8_string&& str) noexcept;
  explicit Exception(const utf8_string&);
  explicit Exception(const char*);

  const utf8_string& What() const noexcept;
  const char* what() const noexcept override;

private:
  utf8_string m_str;
};

} // namespace

#endif
