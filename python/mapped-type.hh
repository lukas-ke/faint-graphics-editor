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

#ifndef FAINT_MAPPED_TYPE_HH
#define FAINT_MAPPED_TYPE_HH

namespace faint{

// Declaration to allow static_assert for default MappedType
class InvalidMappedType{};

// Should be specialized to provide the type name for a Python object
// structure, when parameterized with the corresponding Faint class.
template<typename T>
struct MappedType{
  using PYTHON_TYPE = InvalidMappedType;
};

} // namespace

#endif
