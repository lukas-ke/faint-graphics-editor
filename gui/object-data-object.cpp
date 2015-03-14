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

#include "wx/dataobj.h"
#include "gui/object-data-object.hh"
#include "objects/object.hh"
#include "util/object-util.hh"

namespace faint{

static void delete_objects(objects_t& objects){
  for (auto obj : objects){
    delete obj;
  }
  objects.clear();
}

ObjectDataObject::ObjectDataObject(){
}

ObjectDataObject::ObjectDataObject(const objects_t& objects)
  : m_objects(clone(objects))
{}

ObjectDataObject::~ObjectDataObject(){
  delete_objects(m_objects);
}

void ObjectDataObject::GetAllFormats(wxDataFormat* formats, Direction) const{
  formats[0] = wxDataFormat("FaintObject");
}

bool ObjectDataObject::GetDataHere(const wxDataFormat& format, void *buf) const{
  if (format == wxDataFormat("FaintObject")){
    // Shallow copy - the deep copy is done in SetData, which is
    // called behind the scenes
    memcpy(buf, &(*begin(m_objects)), sizeof(Object*) * m_objects.size());
    return true;
  }
  return false;
}

size_t ObjectDataObject::GetDataSize(const wxDataFormat& f) const{
  if (f == wxDataFormat("FaintObject")){
    return sizeof(Object*) * m_objects.size();
  }
  return 0;
}

size_t ObjectDataObject::GetFormatCount(Direction) const{
  return 1;
}

objects_t ObjectDataObject::GetObjects() const{
  return clone(m_objects);
}

wxDataFormat ObjectDataObject::GetPreferredFormat(Direction) const{
  return wxDataFormat("FaintObject");
}

bool ObjectDataObject::SetData(const wxDataFormat& format, size_t len,
  const void* buf)
{
  if (format != wxDataFormat("FaintObject")){
    return false;
  }

  // Fixme: Need a proper serialized object format.
  delete_objects(m_objects);
  Object** objects = new Object*[len / sizeof(Object*)];
  memcpy(objects, buf, len);
  for (size_t i = 0; i !=  len / sizeof(Object*); i++){
    m_objects.push_back(objects[i]->Clone());
  }
  delete[] objects;
  return true;
}

} // Namespace faint
