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

class CmdFuncs{
public:
  CmdFuncs(const std::function<void()>& Do,
    const std::function<void()>& Undo)
    : Do(Do),
      Undo(Undo)
  {}
  std::function<void()> Do;
  std::function<void()> Undo;
};

class Object {
public:
  Object(const Settings& settings=NullObjectSettings);
  virtual ~Object() = default;
  bool Active() const;
  virtual bool CanRemovePoint() const;
  void ClearActive();
  virtual Object* Clone() const = 0;

  // True if the object is closed, so that points at the opposite ends
  // should be considered adjacent
  virtual bool CyclicPoints() const;
  virtual void Draw(FaintDC&, ExpressionContext&) = 0;
  virtual void DrawMask(FaintDC&) = 0;
  virtual bool Extendable() const;

  // Gets the points in this objects that other objects and tools can
  // snap to
  virtual std::vector<Point> GetAttachPoints() const;

  ObjectId GetId() const;

  // Gets points at which additional points can be inserted
  virtual std::vector<ExtensionPoint> GetExtensionPoints() const;

  // Gets the points that can be moved individually
  virtual std::vector<Point> GetMovablePoints() const;
  const Optional<utf8_string>& GetName() const;
  virtual int GetObjectCount() const;
  virtual Object* GetObject(int);
  virtual const Object* GetObject(int) const;
  virtual std::vector<PathPt> GetPath(const ExpressionContext&) const = 0;
  virtual Point GetPoint(int index) const;
  virtual IntRect GetRefreshRect() const = 0;
  const Settings& GetSettings() const;

  // Gets the points in this object that wish to snap to other points
  // when resizing/moving
  virtual std::vector<Point> GetSnappingPoints() const;
  virtual Tri GetTri() const = 0;
  virtual utf8_string GetType() const = 0;
  virtual bool HitTest(const Point&);
  bool Inactive() const;

  // Insert a point at the given index. Returns a closure for undo,
  // since undo may be more involved than "RemovePoint" (e.g. when
  // splitting a bezier path)
  virtual UndoAddFunc InsertPoint(const Point&, int index);
  virtual bool IsControlPoint(int index) const;
  virtual int NumPoints() const;
  virtual Optional<CmdFuncs> PixelSnapFunc();
  virtual void RemovePoint(int index);
  template<typename T> void Set(const T& s, typename T::ValueType);
  void SetActive(bool active=true);
  void SetName(const Optional<utf8_string>&);
  virtual void SetPoint(const Point&, int index);
  virtual void SetTri(const Tri&) = 0;
  virtual bool ShowSizeBox() const;
  virtual utf8_string StatusString() const;
  bool UpdateSettings(const Settings&);
protected:
  Settings m_settings;
private:
  Object(const Object&);
  bool m_active;
  ObjectId m_id;
  Optional<utf8_string> m_name;
};

template<typename T>
void Object::Set(const T& s, typename T::ValueType v){
  m_settings.Set(s, v);
}

using objects_t = std::vector<Object*>;

} // namespace

#endif
