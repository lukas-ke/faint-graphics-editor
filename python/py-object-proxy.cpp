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

#include "geo/int-rect.hh"
#include "geo/tri.hh"
#include "geo/pathpt.hh"
#include "objects/object.hh"
#include "python/py-object-proxy.hh"
#include "python/py-shape.hh"

namespace faint{

class ObjectProxy : public Object{
  // Wraps an Object and its managing PyObject*
  // to get reference counting when storing in Groups,
  // (or images?)
public:
  ObjectProxy(PyObject* pyObj, Object* obj) :
    m_obj(obj),
    m_pyObj(pyObj)
  {
    Py_INCREF(m_pyObj);
  }

  ~ObjectProxy(){
    faint::py_xdecref(m_pyObj);
  }

  bool CanRemovePoint() const override{
    return m_obj->CanRemovePoint();
  }

  Object* Clone() const override{
    Object* newObj = m_obj->Clone();
    PyObject* newPyObj = create_Shape(newObj);
    return new ObjectProxy(newPyObj, newObj);
  };

  bool CyclicPoints() const override{
    return m_obj->CyclicPoints();
  }

  void Draw(FaintDC& dc, ExpressionContext& ctx) override{
    m_obj->Draw(dc, ctx);
  };
  void DrawMask(FaintDC& dc, ExpressionContext& ctx) override{
    m_obj->DrawMask(dc, ctx);
  };

  bool Extendable() const override{
    return m_obj->Extendable();
  }

  std::vector<Point> GetAttachPoints() const override{
    return m_obj->GetAttachPoints();
  }

  std::vector<ExtensionPoint> GetExtensionPoints() const override{
    return m_obj->GetExtensionPoints();
  }

  std::vector<Point> GetMovablePoints() const override{
    return m_obj->GetMovablePoints();
  }

  int GetObjectCount() const override{
    return m_obj->GetObjectCount();
  }

  Object* GetObject(int i){
    return m_obj->GetObject(i);
  }

  const Object* GetObject(int i) const override{
    return m_obj->GetObject(i);
  }

  std::vector<PathPt> GetPath(const ExpressionContext& ctx) const override{
    return m_obj->GetPath(ctx);
  }

  Point GetPoint(int index) const override{
    return m_obj->GetPoint(index);
  }

  IntRect GetRefreshRect() const override{
    return m_obj->GetRefreshRect();
  }

  // Gets the points in this object that wish to snap to other points
  // when resizing/moving
  std::vector<Point> GetSnappingPoints() const override{
    return m_obj->GetSnappingPoints();
  }

  Tri GetTri() const override{
    return m_obj->GetTri();
  }

  utf8_string GetType() const override{
    return m_obj->GetType();
  }

  bool HitTest(const Point& pt) override{
    return m_obj->HitTest(pt);
  }

  // Insert a point at the given index. Returns a closure for undo,
  // since undo may be more involved than "RemovePoint" (e.g. when
  // splitting a bezier path)
  UndoAddFunc InsertPoint(const Point& pt, int index) override{
    return m_obj->InsertPoint(pt, index);
  }

  bool IsControlPoint(int index) const override{
    return m_obj->IsControlPoint(index);
  }

  int NumPoints() const override{
    return m_obj->NumPoints();
  }

  Optional<CmdFuncs> PixelSnapFunc() override{
    return m_obj->PixelSnapFunc();
  }

  void RemovePoint(int index) override{
    m_obj->RemovePoint(index);
  }

  void SetPoint(const Point& pt, int index) override{
    m_obj->SetPoint(pt, index);
  }

  void SetTri(const Tri& t) override{
    m_obj->SetTri(t);
  }

  bool ShowSizeBox() const override{
    return m_obj->ShowSizeBox();
  }

  utf8_string StatusString() const override{
    return m_obj->StatusString();
  }

  PyObject* GetHolder(){
    return m_pyObj;
  }

private:
  Object* m_obj;
  PyObject* m_pyObj;
};

Object* proxy_shape(PyObject* pyObj){
  Object* obj = shape_get_object(pyObj);
  assert(obj != nullptr);
  return new ObjectProxy(pyObj, obj);
}

PyObject* get_holder(Object* obj){
  auto proxy = dynamic_cast<ObjectProxy*>(obj);
  assert(proxy != nullptr);
  return proxy->GetHolder();
}

} // namespace
