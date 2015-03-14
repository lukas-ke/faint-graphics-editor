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

#ifndef FAINT_INT_RANGE_HH
#define FAINT_INT_RANGE_HH
#include "geo/range.hh"
#include "util/optional.hh"

namespace faint{

class IntRange{
  // An interval for constraining integers. Can extend to infinity in
  // either direction. Closed for finite boundaries (i.e. the boundary
  // is included)
public:
  // Creates an unlimited range, Constrain():s nothing, Has()
  // everything.
  IntRange() = default;

  // A range between min and max. Asserts that min <= max.
  IntRange(const min_t&, const max_t&);

  // Lower bound only
  IntRange(const min_t&);
  IntRange(const max_t&); // Upper bound only

  // Return the value constrained to the range.
  int Constrain(int) const;

  // True if the value is within the range
  bool Has(int) const;

  // True if the interval is within the Range
  bool Has(const Interval&) const;
private:
  Optional<int> m_max;
  Optional<int> m_min;
};

} // namespace

#endif
