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

#ifndef FAINT_HISTOGRAM_HH
#define FAINT_HISTOGRAM_HH
#include <vector>
#include "geo/range.hh"
#include "util/distinct.hh"

namespace faint{

class category_histogram;
using bin_t = Distinct<int, category_histogram, 0>;
using count_t = Distinct<int, category_histogram, 1>;

class Histogram{
public:
  Histogram(const ClosedIntRange&, const bin_t& numBins);
  int Count(const bin_t&) const;
  void Insert(int value);
  void Insert(int value, const count_t&);
  int Max() const;
  int NumBins() const;
private:
  ClosedIntRange m_range;
  std::vector<int> m_data;
};

} // namespace

#endif
