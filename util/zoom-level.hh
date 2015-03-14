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

#ifndef FAINT_ZOOM_LEVEL_HH
#define FAINT_ZOOM_LEVEL_HH
#include "geo/primitive.hh"
#include "util/optional.hh"

namespace faint{

class ZoomLevel{
public:
  enum ChangeType{PREVIOUS, NEXT, DEFAULT};
  ZoomLevel();
  bool At100() const;
  bool AtMax() const;
  bool AtMin() const;
  bool Change(ChangeType);
  int GetPercentage() const;
  coord GetScaleFactor() const;
  bool Next();
  bool Prev();
  void SetApproximate(coord);
private:
  void SetLevel(int);
  int m_zoomLevel;
  Optional<coord> m_scale;
};

}

#endif
