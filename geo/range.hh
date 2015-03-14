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

#ifndef FAINT_RANGE_HH
#define FAINT_RANGE_HH
#include <cassert>
#include <climits> // C++11 (VC): For INT_MAX, std::numeric_limits<int>::max() is not compile-time without constexpr. Not fixed yet in VC2015 preview.
#include "geo/int-size.hh"
#include "util/distinct.hh"

namespace faint{

class category_int_range;
using min_t = Distinct<int, category_int_range, 0>;
using max_t = Distinct<int, category_int_range, 1>;

class Interval{
  // A closed interval of integers, [Lower(), Upper()]
public:
  // Asserts that min <= max
  Interval(const min_t&, const max_t&);
  bool Has(int) const;
  int Lower() const;
  int Mid() const;
  int Upper() const;
  int Size() const;
  int Delta() const;

private:
  int m_min;
  int m_max;
};

class ClosedIntRange{
  // A range that, unlike IntRange, does not allow infinity
public:
  // Asserts that min <= max.
  ClosedIntRange(const min_t&, const max_t&);

  // Return the value constrained to the range.
  int Constrain(int) const;
  double Constrain(double value) const;
  Interval Constrain(const Interval&) const;
  int Delta() const;

  // True if the value is within the range
  bool Has(int) const;
  bool Has(double) const;

  // True if the interval is within the Range
  bool Has(const Interval&) const;
  int Lower() const;
  int Upper() const;
private:
  int m_max;
  int m_min;
};

class BoundedInt{
  // A range between min and max and a value within that interval.
  // Asserts that min <= value <= max
public:
  BoundedInt(const min_t&, int value, const max_t&);

  // A BoundedInt with the middle value
  static BoundedInt Mid(const min_t&, const max_t&);

  // A BoundedInt with the lower value
  static BoundedInt Min(const min_t&, const max_t&);

  int GetValue() const;
  ClosedIntRange GetRange() const;
private:
  ClosedIntRange m_range;
  int m_value;
};

IntSize constrained(const IntSize&, const min_t& w, const min_t& h);

template<int MIN_BOUND, int MAX_BOUND>
class StaticBoundedInt{
  // A template for specifying static integer bounds for function
  // parameters.
public:
  StaticBoundedInt(int value)
    : m_value(value)
  {
    assert(Valid(value));
  }

  StaticBoundedInt()
    : m_value(MIN_BOUND)
  {}

  int GetValue() const{
    return m_value;
  }

  static bool Valid(int value){
    return MIN_BOUND <= value && value <= MAX_BOUND;
  }

  static_assert(MIN_BOUND < MAX_BOUND, "Invalid template range for BoundedInteger");
  // Using static integers with in-class initialization caused linker
  // error in template function as_closed_range with g++ 4.7.2 when
  // optimization off (flags -g -O0).
  //
  // Using enum-hack instead for now.
  enum {min_allowed=MIN_BOUND};
  enum {max_allowed=MAX_BOUND};
  using value_type = int;
private:
  int m_value;
};

template<int MIN_BOUND>
using LowerBoundedInt = StaticBoundedInt<MIN_BOUND, INT_MAX>;

template<int MIN_BOUND, int MAX_BOUND>
class StaticBoundedInterval{
  // A template for specifying static integer bounds for function
  // parameter intervals.
public:
  StaticBoundedInterval(const Interval& interval)
    : m_interval(interval)
  {
    assert(Valid(interval));
  }

  StaticBoundedInterval()
    : m_interval(min_t(MIN_BOUND), max_t(MIN_BOUND))
  {}

  Interval GetInterval() const{
    return m_interval;
  }

  int GetMax() const{
    return m_interval.Upper();
  }

  int GetMin() const{
    return m_interval.Lower();
  }

  static bool Valid(const Interval& interval){
    return MIN_BOUND <= interval.Lower() && interval.Upper() <= MAX_BOUND;
  }

  static_assert(MIN_BOUND < MAX_BOUND,
    "Invalid template range for BoundedInterval");

  // Using static integers with in-class initialization caused linker
  // error in template function as_closed_range with g++ 4.7.2 when
  // optimization off (flags -g -O0).
  //
  // Using enum-hack instead for now.
  enum {min_allowed=MIN_BOUND};
  enum {max_allowed=MAX_BOUND};
  using value_type = int;

private:
  Interval m_interval;
};

// Creates a ClosedIntRange.
// Note: Indifferent to which value is greater (unlike ClosedIntRange
// constructor)
ClosedIntRange make_closed_range(int, int);

// Create an Interval.
// Note: Indifferent to which value is greater (unlike Interval
// constructor)
Interval make_interval(int v0, int v1);

// Get the value at the ratio between the maximum and minimum for the
// range type.
template<typename RangeType>
int value_at(double fraction){
  return static_cast<typename RangeType::value_type>((RangeType::max_allowed -
      RangeType::min_allowed) * fraction) + RangeType::min_allowed;
}

// Get the interval between the ratios of the maximum and minimum for
// the range type.
template<typename RangeType>
Interval fractional_interval(double fraction1, double fraction2){
  return make_interval(value_at<RangeType>(fraction1),
    value_at<RangeType>(fraction2));
}

template<typename RangeType>
RangeType fractional_bounded_interval(double fraction1, double fraction2){
  return RangeType(fractional_interval<RangeType>(fraction1, fraction2));
}

// Create a regular ClosedIntRange from the template range type.
// Ignores values, uses only the static boundaries.
template<typename RangeType>
ClosedIntRange as_closed_range(){
  return ClosedIntRange(min_t(RangeType::min_allowed),
    max_t(RangeType::max_allowed));
}

// Returns the value constrained between the minimum and maximum.
template<typename T>
T constrained(const typename Boundary<T>::Min& minVal,
  T value,
  const typename Boundary<T>::Max& maxVal){
  return std::max(minVal.Get(), std::min(value, maxVal.Get()));
}

} // namespace

#endif
