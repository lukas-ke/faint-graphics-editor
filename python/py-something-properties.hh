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

#ifndef FAINT_PY_SOMETHING_PROPERTIES_HH
#define FAINT_PY_SOMETHING_PROPERTIES_HH

// Hand-written (i.e. not generated) properties for SmthType

namespace faint{

/* property: "The object's name" */
struct Smth_name{
  static Optional<utf8_string> Get(const BoundObject<Object>& self){
    return self.obj->GetName();
  }

  static void Set(const BoundObject<Object>& self,
    const Optional<utf8_string>& name)
  {
    auto newName(empty_to_unset(name));
    if (self.obj->GetName() != newName){
      self.ctx->RunCommand(self, set_object_name_command(self.obj, newName));
    }
  }
};

/* property: "The object's angle in radians" */
struct Smth_angle_rad{
  static coord Get(const BoundObject<Object>& self){
    return self.obj->GetTri().GetAngle().Rad();
  }

  static void Set(const BoundObject<Object>& self, coord angle){
    Object* obj = self.obj;
    Tri oldTri = obj->GetTri();
    Tri newTri = rotated(oldTri, Angle::Rad(angle) - oldTri.GetAngle(),
      center_point(oldTri));

    self.ctx->RunCommand(self,
      tri_command(obj, New(newTri), Old(oldTri)));
  }
};

/* property: "The object's angle in degrees" */
struct Smth_angle_deg{
  static coord Get(const BoundObject<Object>& self){
    return self.obj->GetTri().GetAngle().Deg();
  }

  static void Set(const BoundObject<Object>& self, coord angle){
    Object* obj = self.obj;
    Tri oldTri = obj->GetTri();
    Tri newTri = rotated(oldTri, Angle::Deg(angle) - oldTri.GetAngle(),
      center_point(oldTri));

    self.ctx->RunCommand(self,
      tri_command(obj, New(newTri), Old(oldTri)));
  }
};

/* property: "The bounds of this object, specified by three points" */
struct Smth_tri{
  static Tri Get(const BoundObject<Object>& self){
    return self.obj->GetTri();
  }

  static void Set(const BoundObject<Object>& self, const Tri& tri){
    self.ctx->RunCommand(self,
      tri_command(self.obj, New(tri), Old(self.obj->GetTri())));
  }
};

/* property: "The position of the object, defined as P0 of its Tri." */
struct Smth_pos{
  static Point Get(const BoundObject<Object>& self){
    return self.obj->GetTri().P0();
  }

  static void Set(const BoundObject<Object>& self, const Point& pos){
    Tri oldTri = self.obj->GetTri();
    Point diff = pos - oldTri.P0();
    Tri newTri = translated(oldTri, diff.x, diff.y);

    self.ctx->RunCommand(self,
      tri_command(self.obj, New(newTri), Old(oldTri)));
  }
};

/* property: "A string indicating the type of the object." */
struct Smth_type{
  static utf8_string Get(const BoundObject<Object>& self){
    return self.obj->GetType();
  }

  static void Set(const BoundObject<Object>&, const utf8_string&){
    throw ValueError("Object type can not be changed.");
  }
};

} // namespace

#endif
