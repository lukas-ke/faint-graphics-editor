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

#ifndef FAINT_ONE_INSTANCE_HH
#define FAINT_ONE_INSTANCE_HH
#include <memory>
#include <string>
#include "util/distinct.hh"

namespace faint{

class FileList;

class FaintInstance{
public:
  virtual ~FaintInstance() = default;
  virtual bool AllowStart() const = 0;
};

using allow_server = Distinct<bool, FaintInstance, 0>;
using force_start = Distinct<bool, FaintInstance, 1>;

std::unique_ptr<FaintInstance> create_faint_instance(const FileList& cmdLineFiles,
  const allow_server&,
  const force_start&,
  const std::string& port);

} // namespace

#endif
