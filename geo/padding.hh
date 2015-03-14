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

#ifndef FAINT_PADDING_HH
#define FAINT_PADDING_HH

namespace faint{

class IntSize;

class Padding{
public:
  // Pad all sides by the specified value
  static Padding All(int);
  static Padding Right(int);
  static Padding Bottom(int);
  // Pad all sides by half the value
  static Padding Divide(int);

  static Padding None();
  IntSize GetSize() const;
  Padding operator+(const Padding&) const;

  int bottom;
  int left;
  int right;
  int top;

private:
  Padding(int, int, int, int);
};

} // namespace

#endif
