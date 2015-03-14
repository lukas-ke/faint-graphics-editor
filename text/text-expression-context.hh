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

#ifndef FAINT_TEXT_EXPRESSION_CONTEXT_HH
#define FAINT_TEXT_EXPRESSION_CONTEXT_HH
#include "geo/calibration.hh"
#include "text/utf8-string.hh"
#include "util/template-fwd.hh"

namespace faint{
class Object;

class ExpressionContext{
// Context for evaluting an ExpressionTree.
public:
  virtual ~ExpressionContext() = default;
  virtual Optional<Calibration> GetCalibration() const = 0;
  virtual const Object* GetObject(const utf8_string& name) const = 0;
};

} // namespace

#endif
