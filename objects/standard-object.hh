// -*- coding: us-ascii-unix -*-
// Copyright 2015 Lukas Kemmer
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

#ifndef FAINT_STANDARD_OBJECT_HH
#define FAINT_STANDARD_OBJECT_HH
#include "objects/object.hh"

namespace faint{

class StandardObject : public Object {
public:
  StandardObject(const Settings& settings=NullObjectSettings);
  bool Active() const override;
  bool CanRemovePoint() const override;

  // True if the object is closed, so that points at the opposite ends
  // should be considered adjacent
  bool CyclicPoints() const override;
  bool Extendable() const override;

  // Gets the points in this objects that other objects and tools can
  // snap to
  std::vector<Point> GetAttachPoints() const override;

  ObjectId GetId() const override;

  // Gets points at which additional points can be inserted
  std::vector<ExtensionPoint> GetExtensionPoints() const override;

  // Gets the points that can be moved individually
  std::vector<Point> GetMovablePoints() const override;
  const Optional<utf8_string>& GetName() const override;
  int GetObjectCount() const override;
  Object* GetObject(int) override;
  const Object* GetObject(int) const override;
  Point GetPoint(int index) const override;
  const Settings& GetSettings() const override;
  Settings& GetSettings() override;
  // Gets the points in this object that wish to snap to other points
  // when resizing/moving
  std::vector<Point> GetSnappingPoints() const override;
  bool HitTest(const Point&) override;

  // Insert a point at the given index. Returns a closure for undo,
  // since undo may be more involved than "RemovePoint" (e.g. when
  // splitting a bezier path)
  UndoAddFunc InsertPoint(const Point&, int index) override;
  bool IsControlPoint(int index) const override;
  int NumPoints() const override;
  Optional<CmdFuncs> PixelSnapFunc() override;
  void RemovePoint(int index) override;
  void SetActive(bool active=true) override;
  void SetName(const Optional<utf8_string>&) override;
  void SetPoint(const Point&, int index) override;
  bool ShowSizeBox() const override;
  utf8_string StatusString() const override;
  bool UpdateSettings(const Settings&) override;

protected:
  Settings m_settings;

private:
  StandardObject(const StandardObject&);
  bool m_active;
  ObjectId m_id;
  Optional<utf8_string> m_name;
};

} // namespace

#endif
