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

#include "geo/bezier.hh"
#include "geo/geo-func.hh"
#include "geo/int-rect.hh"
#include "geo/measure.hh"
#include "geo/points.hh"
#include "objects/object.hh"
#include "objects/objpath.hh"
#include "rendering/faint-dc.hh"
#include "text/utf8-string.hh"
#include "util/object-util.hh"
#include "util/setting-id.hh"
#include "util/setting-util.hh"
#include "util/settings.hh"

namespace faint{

class ObjPath : public Object{
public:
  ObjPath(const Points& points, const Settings& settings)
    : Object(with_point_editing(settings, start_enabled(false))),
      m_points(points),
      m_tri(points.GetTri())
  {}

  bool CyclicPoints() const override{
    return true;
  }

  Object* Clone() const override{
    return new ObjPath(*this);
  }

  void Draw(FaintDC& dc, ExpressionContext&) override{
    dc.Path(m_points.GetPoints(m_tri), m_settings);
  }

  void DrawMask(FaintDC& dc) override{
    dc.Path(m_points.GetPoints(m_tri), mask_settings_fill(m_settings));
  }

  std::vector<Point> GetAttachPoints() const override{
    return std::vector<Point>();
  }

  std::vector<ExtensionPoint> GetExtensionPoints() const override{
    if (m_points.Empty()){
      return {};
    }
    auto pathPts = m_points.GetPoints(m_tri);

    Point current = pathPts.front().p;
    std::vector<ExtensionPoint> extensionPoints;
    int i = 0;
    for (const auto& pt : pathPts){
      pt.Visit(
        [&](const ArcTo& ap){
          current = ap.p;
          i++;
        },
        [](const Close&){},
        [&](const CubicBezier& bezier){
          extensionPoints.push_back({bezier_point(0.5, current, bezier), i});
          current = bezier.p;
          i += 3;
        },
        [&](const LineTo& to){
          current = to.p;
          i += 1;
        },
        [&](const MoveTo& to){
          current = to.p;
          i += 1;
        });
    }
    return extensionPoints;
  }

  std::vector<Point> GetMovablePoints() const override{
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    std::vector<Point> movablePts;
    movablePts.reserve(pathPts.size());
    for (const PathPt& pt : pathPts){
      if (pt.IsCubicBezier()){
        movablePts.push_back(pt.p);
        movablePts.push_back(pt.c);
        movablePts.push_back(pt.d);
      }
      else if (pt.IsLine()){
        movablePts.push_back(pt.p);
      }
      else if (pt.IsMove()){
        movablePts.push_back(pt.p);
      }
    }
    return movablePts;
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return m_points.GetPoints(m_tri);
  }

  Point GetPoint(int index) const override{
    return GetMovablePoints()[to_size_t(index)]; // Fixme: Slow
  }

  IntRect GetRefreshRect() const override{
    return floored(bounding_rect(m_tri, m_settings));
  }

  Tri GetTri() const override{
    return m_tri;
  }

  utf8_string GetType() const override{
    return "Path";
  }

  void InsertPoint(const Point& pt, int index) override{
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    auto p0 = pathPts.at(index - 1); // Fixme: Check bounds
    auto p = pathPts.at(index);
    if (p.type == PathPt::Type::CubicBezier){
      CubicBezier b(p.p, p.c, p.d);
      auto bs = in_twain(p0.p, b);
      bs.first.p = pt;
      m_points.RemovePointRaw(index);
      m_points.InsertPointRaw(bs.second, index);
      m_points.InsertPointRaw(bs.first, index);
    }
  }

  bool IsControlPoint(int index) const override{
    assert(index >= 0);
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    int at = 0;
    // Fixme: Duplicates SetPoint
    for (size_t i = 0; i != pathPts.size(); i++){
      const PathPt& pt = pathPts[i];
      if (pt.IsCubicBezier()){
        if (index <= at + 2){
          if (index == at){
            return false;
          }
          else if (index == at + 1){
            return true;
          }
          else if (index == at + 2){
            return true;
          }
          assert(false);
        }
        at += 3;
      }
      else if (pt.IsLine()){
        if (index == at){
          return false;
        }
        at += 1;
      }
      else if (pt.IsMove()){
        if (index == at){
          return false;
        }
        at += 1;
      }
      // Fixme: What of arc etc? :-x Use visit.
    }
    return false;
  }

  int NumPoints() const override{
    return resigned(GetMovablePoints().size()); // Fixme: slow.
  }

  void RemovePoint(int index) override{
    // Fixme: When used for undo, this will deform the path.
    assert(index >= 0);
    m_points.RemovePointRaw(index);
  }

  void SetPoint(const Point& pt, int index) override{
    assert(index >= 0);
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    int at = 0;

    for (size_t i = 0; i != pathPts.size(); i++){
      const PathPt& oldPt = pathPts[i];
      if (oldPt.IsCubicBezier()){
        if (index <= at + 2){
          PathPt copy(oldPt);
          if (index == at){
            copy.p = pt;
          }
          else if (index == at + 1){
            copy.c = pt;
          }
          else if (index == at + 2){
            copy.d = pt;
          }
          else{
            assert(false);
          }
          m_points.SetPoint(m_tri, copy, resigned(i));
          break;
        }
        at += 3;
      }
      else if (oldPt.IsLine()){
        if (index == at){
          m_points.SetPoint(m_tri, PathPt::LineTo(pt), resigned(i));
          break;
        }
        at += 1;
      }
      else if (oldPt.IsMove()){
        if (index == at){
          m_points.SetPoint(m_tri, PathPt::MoveTo(pt), resigned(i));
          break;
        }
        at += 1;
      }
    }
    SetTri(m_points.GetTri()); // Fixme
  }

  void SetTri(const Tri& t) override{
    m_tri = t;
  }

private:
  // For clone
  ObjPath(const ObjPath& other) :
    Object(other.m_settings),
    m_points(other.m_points),
    m_tri(other.GetTri())
  {}

  Points m_points;
  Tri m_tri;
};

Object* create_path_object(const Points& points, const Settings& settings){
  return new ObjPath(points, without(settings, ts_ClosedPath));
}

} // namespace
