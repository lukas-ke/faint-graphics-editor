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

#include <algorithm>
#include <cassert>
#include <iterator> // back_inserter
#include "geo/int-rect.hh"
#include "geo/pathpt.hh"
#include "geo/rect.hh"
#include "geo/scale.hh"
#include "geo/tri.hh"
#include "objects/object.hh"
#include "objects/objcomposite.hh"
#include "text/utf8-string.hh"
#include "util/setting-id.hh"

namespace faint{

static void update_objects(const Tri& tri,
  const Tri& origTri, const objects_t& objects,
  const std::vector<Tri>& objTris)
{
  Adj a = get_adj(origTri, tri);
  coord ht = fabs(origTri.P2().y - origTri.P1().y);
  for (size_t objNum = 0; objNum != objects.size(); objNum++){
    Tri temp = objTris[objNum];

    Point p0(temp.P0());
    Point p1(temp.P1());
    Point p2(temp.P2());

    if (a.scale_x != 0 && ht != 0){
      // Cannot skew objects scaled to nothing or with a height of zero.
      p0.x += a.skew * (fabs(ht - p0.y) / ht) / a.scale_x;
      p1.x += a.skew * (fabs(ht - p1.y) / ht) / a.scale_x;
      p2.x += a.skew * (fabs(ht - p2.y) / ht) / a.scale_x;
    }
    temp = translated(Tri(p0, p1, p2), origTri.P0().x + a.tr_x, origTri.P0().y +
      a.tr_y);
    temp = scaled(temp, Scale(a.scale_x, a.scale_y), tri.P3());
    temp = rotated(temp, tri.GetAngle(), tri.P3());
    objects[objNum]->SetTri(temp);
  }
}

class ObjComposite : public Object{
public:
  ObjComposite(const objects_t& objects, Ownership ownership)
    : m_objects(objects),
      m_ownership(ownership)
  {
    m_settings.Set(ts_AlignedResize, false);
    assert(!objects.empty());
    Tri t = objects.front()->GetTri();
    Point minPt = min_coords(t.P0(), t.P1(), t.P2(), t.P3());
    Point maxPt = max_coords(t.P0(), t.P1(), t.P2(), t.P3());
    for (const Object* obj : m_objects){
      t = obj->GetTri();
      m_objTris.push_back(t);
      minPt = min_coords(minPt, min_coords(t.P0(), t.P1(), t.P2(), t.P3()));
      maxPt = max_coords(maxPt, max_coords(t.P0(), t.P1(), t.P2(), t.P3()));
    }

    SetTri(tri_from_rect(Rect(minPt, maxPt)));
    m_origTri = GetTri();
    for (Tri& objTri : m_objTris){
      objTri = Tri(objTri.P0() - m_origTri.P0(), objTri.P1() - m_origTri.P0(),
        objTri.P2() - m_origTri.P0());
    }
    update_objects(GetTri(), m_origTri, m_objects, m_objTris);
  }

  ~ObjComposite(){
    if (m_ownership == Ownership::OWNER){
      for (Object* obj : m_objects){
        delete obj;
      }
    }
  }

  void Draw(FaintDC& dc, ExpressionContext& ctx) override{
    for (Object* obj : m_objects){
      obj->Draw(dc, ctx);
    }
  }

  void DrawMask(FaintDC& dc) override{
    for (Object* obj : m_objects){
      obj->DrawMask(dc);
    }
  }

  IntRect GetRefreshRect() const override{
    assert(!m_objects.empty());
    IntRect r(m_objects[0]->GetRefreshRect());
    for (size_t i = 1; i != m_objects.size(); i++){
      r = union_of(r, m_objects[i]->GetRefreshRect());
    }
    return r;
  }

  utf8_string GetType() const override{
    return "Group";
  }

  Object* Clone() const override{
    return new ObjComposite(*this);
  }

  int GetObjectCount() const override{
    return resigned(m_objects.size());
  }

  Object* GetObject(int index) override{
    return m_objects[to_size_t(index)];
  }

  std::vector<PathPt> GetPath(const ExpressionContext& ctx) const override{
    std::vector<PathPt> path;
    for (Object* object : m_objects){
      std::vector<PathPt> subPath(object->GetPath(ctx));
      for (PathPt pt : subPath){
        path.push_back(pt);
      }
    }
    return path;
  }

  const Object* GetObject(int index) const override{
    return m_objects[to_size_t(index)];
  }

  std::vector<Point> GetSnappingPoints() const override{
    // Fixme: Why only top left?
    return { bounding_rect(m_tri).TopLeft() };
  }

  Tri GetTri() const override{
    return m_tri;
  }

  // Fixme: This is probably horrendously slow for complex groups
  std::vector<Point> GetAttachPoints() const override{
    std::vector<Point> points;
    for (const Object* obj : m_objects){
      std::vector<Point> current(obj->GetAttachPoints());
      std::copy(begin(current), end(current), std::back_inserter(points));
    }
    return points;
  }

  void SetTri(const Tri& tri) override{
    assert(valid(tri));
    m_tri = tri;
    update_objects(m_tri, m_origTri, m_objects, m_objTris);
  }

private:
  ObjComposite(const ObjComposite& other)
    : Object(other.GetSettings()),
      m_tri(other.m_tri),
      m_origTri(other.m_origTri),
      m_objTris(other.m_objTris),
      m_ownership(Ownership::OWNER)
  {
    for (const Object* obj : other.m_objects){
      m_objects.push_back(obj->Clone());
    }
  }

  objects_t m_objects;
  Tri m_tri;
  Tri m_origTri;
  std::vector<Tri> m_objTris;
  Ownership m_ownership;
};

Object* create_composite_object(const objects_t& objects, Ownership owner){
  return new ObjComposite(objects, owner);
}

bool is_composite(Object* obj){
  return dynamic_cast<ObjComposite*>(obj) != nullptr;
}

} // namespace
