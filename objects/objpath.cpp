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
#include "geo/pathpt-iter.hh"
#include "geo/pixel-snap.hh"
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

  void DrawMask(FaintDC& dc, ExpressionContext&) override{
    dc.Path(m_points.GetPoints(m_tri), mask_settings_fill(m_settings));
  }

  std::vector<Point> GetAttachPoints() const override{
    std::vector<Point> attachPoints;
    auto pts = m_points.GetPoints(m_tri);
    if (pts.empty()){
      return {};
    }

    const Point first = pts.front().p;
    Point prev = first;

    for_each_pt(pts,
      [&](const ArcTo& arc){
        // Fixme: Verify that p is end-point
        prev = arc.p;
      },
      [&](const Close&){
        attachPoints.push_back(mid_point(prev, first));
      },
      [&](const CubicBezier& bezier){
        attachPoints.push_back(bezier_point(0.5, prev, bezier));
        attachPoints.push_back(bezier.p);
        prev = bezier.p;
      },
      [&](const LineTo& to){
        attachPoints.push_back(mid_point(prev, to.p));
        attachPoints.push_back(to.p);
        prev = to.p;
      },
      [&](const MoveTo& to){
        attachPoints.push_back(to.p);
        prev = to.p;
      });
    return attachPoints;
  }
  std::vector<ExtensionPoint> GetExtensionPoints() const override{
    if (m_points.Empty()){
      return {};
    }
    auto pathPts = m_points.GetPoints(m_tri);

    Point current = pathPts.front().p;
    std::vector<ExtensionPoint> extensionPoints;
    int i = 0;
    for_each_pt(pathPts,
      [&](const ArcTo& ap){
        current = ap.p;
        i++;
      },
      [&](const Close&){
        extensionPoints.push_back({mid_point(current, pathPts.front().p), i});
        i++;
      },
      [&](const CubicBezier& bezier){
        extensionPoints.push_back({bezier_point(0.5, current, bezier), i});
        current = bezier.p;
        i += 3;
      },
      [&](const LineTo& to){
        extensionPoints.push_back({mid_point(current, to.p), i});
        current = to.p;
        i += 1;
      },
      [&](const MoveTo& to){
        current = to.p;
        i += 1;
      });
    return extensionPoints;
  }

  std::vector<Point> GetMovablePoints() const override{
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    std::vector<Point> movablePts;
    movablePts.reserve(pathPts.size());

    for_each_pt(pathPts,
      [](const ArcTo&){},
      [](const Close&){},
      [&](const CubicBezier& bezier){
        movablePts.push_back(bezier.p);
        movablePts.push_back(bezier.c);
        movablePts.push_back(bezier.d);
      },
      [&](const LineTo& lineTo){
        movablePts.push_back(lineTo.p);
      },
      [&](const MoveTo& moveTo){
        movablePts.push_back(moveTo.p);
      });
    return movablePts;
  }

  std::vector<PathPt> GetPath(const ExpressionContext&) const override{
    return m_points.GetPoints(m_tri);
  }

  Point GetPoint(int index) const override{
    return GetMovablePoints()[to_size_t(index)];
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

  UndoAddFunc InsertPoint(const Point& pt, int index) override{
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    auto p0 = pathPts.at(index - 1); // Fixme: Check bounds
    return pathPts.at(index).Visit(
      [](const ArcTo&) -> std::function<void()>{
        assert(false); // Not implemented
        return [](){};
      },
      [&](const Close&) -> std::function<void()>{
        m_points.InsertPointRaw(LineTo(pt), index);
        return [=](){
          m_points.RemovePointRaw(index);
        };
      },
      [&](const CubicBezier& b) -> std::function<void()>{
        auto bs = in_twain(p0.p, b);
        bs.first.p = pt;
        auto old = m_points.GetPoints(m_tri)[index];
        m_points.RemovePointRaw(index);
        m_points.InsertPointRaw(bs.second, index);
        m_points.InsertPointRaw(bs.first, index);

        return [=](){
          m_points.RemovePointRaw(index);
          m_points.RemovePointRaw(index);
          m_points.InsertPointRaw(old, index);
        };
      },
      [&](const LineTo&) -> std::function<void()>{
        m_points.InsertPointRaw(LineTo(pt), index);
        return [](){};
      },
      [](const MoveTo&) -> std::function<void()>{
        assert(false); // Not implemented
        return [](){};
      });
  }

  bool IsControlPoint(int index) const override{
    assert(index >= 0);
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);
    int at = 0;

    auto unaryNonControlPoint =
      [&](){
        if (index == at){
          return true;
        }
        at += 1;
        return false;
      };

    bool controlPoint = false;
    until_true_pt(pathPts,
      [&](const ArcTo&){return unaryNonControlPoint();},
      [&](const Close&){return unaryNonControlPoint();},
      [&](const CubicBezier&){
        if (index <= at + 2){
          // The first bezier point is the end point, the remaining
          // two are control points
          controlPoint = index != at;
          return true;
        }
        at += 3;
        return false;
      },
      [&](const LineTo&){return unaryNonControlPoint();},
      [&](const MoveTo&){return unaryNonControlPoint();});

    return controlPoint;
  }

  int NumPoints() const override{
    return resigned(GetMovablePoints().size());
  }

  Optional<CmdFuncs> PixelSnapFunc(){
    return {CmdFuncs(
      [this](){
        m_points = pixel_snap(m_points, m_settings.Get(ts_LineWidth));
        m_tri = m_points.GetTri();
      },
      [oldPoints=m_points, oldTri=m_tri, this](){
        m_points = oldPoints;
        m_tri = oldTri;

      })};
  }

  void RemovePoint(int index) override{
    // Fixme: When used for undo, this will deform the path.
    assert(index >= 0);
    m_points.RemovePointRaw(index);
  }

  void SetPoint(const Point& pt, int index) override{
    assert(index >= 0);
    std::vector<PathPt> pathPts = m_points.GetPoints(m_tri);

    // Current control-point index. Certain PathPts have control points,
    // so this can be greater than the PathPt number.
    int at = 0;

    for (size_t i = 0; i != pathPts.size(); i++){
      const bool done = pathPts[i].Visit(
        [&](const ArcTo&){
          // ArcTo has no handles
          return false;
        },
        [&](const Close&){
          // Close has no handles
          return false;
        },
        [&](const CubicBezier& b){
          if (index == at){
            m_points.SetPoint(m_tri, CubicBezier(pt, b.c, b.d), resigned(i));
            return true;
          }
          else if (index == at + 1){
            m_points.SetPoint(m_tri, CubicBezier(b.p, pt, b.d), resigned(i));
            return true;
          }
          else if (index == at + 2){
            m_points.SetPoint(m_tri, CubicBezier(b.p, b.c, pt), resigned(i));
            return true;
          }
          else{
            at += 3;
            return false;
          }
        },
        [&](const LineTo&){
          if (index == at){
            m_points.SetPoint(m_tri, PathPt::LineTo(pt), resigned(i));
            return true;
          }
          at += 1;
          return false;
        },
        [&](const MoveTo&){
          if (index == at){
            m_points.SetPoint(m_tri, PathPt::MoveTo(pt), resigned(i));
            return true;
          }
          at += 1;
          return false;
        });
      if (done){
        break;
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
