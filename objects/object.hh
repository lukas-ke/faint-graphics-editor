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

#ifndef FAINT_OBJECT_HH
#define FAINT_OBJECT_HH
#include <functional>
#include <vector>
#include "geo/point.hh"
#include "util/id-types.hh"
#include "util/optional.hh"
#include "util/settings.hh"

namespace faint{

class ExpressionContext;
class FaintDC;
class IntRect;
class PathPt;
class Point;
class Tri;

extern const Settings NullObjectSettings;
extern const Color mask_outside;
extern const Color mask_fill;
extern const Color mask_no_fill;
extern const Color mask_edge;

class ExtensionPoint{
public:
  // The clickable position for extending the object with another
  // point.
  Point pos;

  // The index used for referring to a point after it has been added
  // at this extension.
  int futureIndex;

  operator Point() const{
    return pos;
  }
};

using UndoAddFunc = std::function<void()>;
using UndoFunc = std::function<void()>;
using CmdFunc = std::function<void()>;

class CmdFuncs{
public:
  CmdFuncs(const CmdFunc& Do,
    const CmdFunc& Undo)
    : Do(Do),
      Undo(Undo)
  {}
  CmdFunc Do;
  CmdFunc Undo;
};

class Object {
public:
  Object() = default;
  virtual ~Object() = default;
  virtual bool Active() const = 0;
  virtual bool CanRemovePoint() const = 0;
  void ClearActive();
  virtual Object* Clone() const = 0;

  // True if the object is closed, so that points at the opposite ends
  // should be considered adjacent
  virtual bool CyclicPoints() const = 0;
  virtual void Draw(FaintDC&, ExpressionContext&) = 0;
  virtual void DrawMask(FaintDC&, ExpressionContext&) = 0;
  virtual bool Extendable() const = 0;

  // Gets the points in this objects that other objects and tools can
  // snap to
  virtual std::vector<Point> GetAttachPoints() const = 0;

  virtual ObjectId GetId() const = 0;

  // Gets points at which additional points can be inserted
  virtual std::vector<ExtensionPoint> GetExtensionPoints() const = 0;

  // Gets the points that can be moved individually
  virtual std::vector<Point> GetMovablePoints() const = 0;
  virtual const Optional<utf8_string>& GetName() const = 0;
  virtual int GetObjectCount() const = 0;
  virtual Object* GetObject(int) = 0;
  virtual const Object* GetObject(int) const = 0;
  virtual std::vector<PathPt> GetPath(const ExpressionContext&) const = 0;
  virtual Point GetPoint(int index) const = 0;
  virtual IntRect GetRefreshRect() const = 0;
  virtual const Settings& GetSettings() const = 0;
  virtual Settings& GetSettings() = 0;

  // Gets the points in this object that wish to snap to other points
  // when resizing/moving
  virtual std::vector<Point> GetSnappingPoints() const = 0;
  virtual Tri GetTri() const = 0;
  virtual utf8_string GetType() const = 0;
  virtual bool HitTest(const Point&) = 0;
  bool Inactive() const;

  // Insert a point at the given index. Returns a closure for undo,
  // since undo may be more involved than "RemovePoint" (e.g. when
  // splitting a bezier path)
  virtual UndoAddFunc InsertPoint(const Point&, int index) = 0;
  virtual bool IsControlPoint(int index) const = 0;
  virtual int NumPoints() const = 0;
  virtual Optional<CmdFuncs> PixelSnapFunc() = 0;
  virtual void RemovePoint(int index) = 0;
  virtual void SetActive(bool active=true) = 0;
  virtual void SetName(const Optional<utf8_string>&) = 0;
  virtual void SetPoint(const Point&, int index) = 0;
  virtual void SetTri(const Tri&) = 0;
  virtual bool ShowSizeBox() const = 0;
  virtual utf8_string StatusString() const = 0;
  virtual bool UpdateSettings(const Settings&) = 0;
  template<typename T> void Set(const T& s, typename T::ValueType);
};

template<typename T>
void Object::Set(const T& s, typename T::ValueType v){
  GetSettings().Set(s, v);
}

using objects_t = std::vector<Object*>;

} // namespace

#endif
