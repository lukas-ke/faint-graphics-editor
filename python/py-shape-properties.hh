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

#ifndef FAINT_PY_SHAPE_PROPERTIES_HH
#define FAINT_PY_SHAPE_PROPERTIES_HH

namespace faint{

// Hand-written (i.e. not generated) properties for ShapeType

/* property: "The object's name" */
struct Shape_name{
  static Optional<utf8_string> Get(Object& self){
    return self.GetName();
  }

  static void Set(Object& self, const Optional<utf8_string>& name){
    self.SetName(empty_to_unset(name));
  }
};

/* property: "The object's angle in radians" */
struct Shape_angle_rad{
  static coord Get(Object& self){
    return self.GetTri().GetAngle().Rad();
  }

  static void Set(Object& self, coord angle){
    Tri oldTri = self.GetTri();
    Tri newTri = rotated(oldTri, Angle::Rad(angle) - oldTri.GetAngle(),
      center_point(oldTri));
    self.SetTri(newTri);
  }
};

/* property: "The object's angle in degrees" */
struct Shape_angle_deg{
  static coord Get(Object& self){
    return self.GetTri().GetAngle().Deg();
  }

  static void Set(Object& self, coord angle){
    Tri oldTri = self.GetTri();
    Tri newTri = rotated(oldTri, Angle::Deg(angle) - oldTri.GetAngle(),
      center_point(oldTri));

    self.SetTri(newTri);
  }
};

/* property: "The bounds of this object, specified by three points" */
struct Shape_tri{
  static Tri Get(const Object& self){
    return self.GetTri();
  }

  static void Set(Object& self, const Tri& tri){
    self.SetTri(tri);
  }
};


/* property: "The position of the object, defined as P0 of its Tri." */
struct Shape_pos{
  static Point Get(Object& self){
    return self.GetTri().P0();
  }

  static void Set(Object& self, const Point& pos){
    Tri oldTri = self.GetTri();
    Point diff = pos - oldTri.P0();
    Tri newTri = translated(oldTri, diff.x, diff.y);
    self.SetTri(newTri);
  }
};

/* property: "A string indicating the type of the object." */
struct Shape_type{
  static utf8_string Get(Object& self){
    return self.GetType();
  }

  static void Set(Object&, const utf8_string&){
    throw ValueError("Object type can not be changed.");
  }
};

} // namespace

#endif
