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
#include "commands/command.hh"
#include "commands/group-objects-cmd.hh"
#include "objects/objcomposite.hh"
#include "objects/object.hh"
#include "text/formatting.hh"
#include "util/iter.hh"
#include "util/object-util.hh"

namespace faint{

class GroupObjectsCommand : public Command {
public:
  GroupObjectsCommand(const objects_t& objects, const select_added& select)
    : Command(CommandType::OBJECT),
      m_group(create_composite_object(objects, Ownership::LOANER)),
      m_objects(objects),
      m_select(select.Get())
  {}

  ~GroupObjectsCommand(){
    delete m_group;
  }

  void Do(CommandContext& context) override{
    // Initialize on the first run
    if (m_objectDepths.empty()){
      for (const Object* obj : m_objects){
        m_objectDepths.push_back(context.GetObjectZ(obj));
      }
    }

    // Traverse and remove objects, adding them instead as a group
    int depth = 0;
    for (int i = 0; i!= m_group->GetObjectCount(); i++){
      Object* obj = m_group->GetObject(i);
      depth = std::max(depth, context.GetObjectZ(obj));
      context.Remove(obj);
    }
    context.Add(m_group, depth, select_added(m_select), deselect_old(false));
  }

  utf8_string Name() const override{
    return space_sep("Group", get_collective_type(m_objects));
  }

  void Undo(CommandContext& context) override{
    context.Remove(m_group);
    for (size_t i = 0; i!= m_objects.size(); i++){
      context.Add(m_objects[i], m_objectDepths[i], select_added(false),
        deselect_old(false));
    }
  }

  // Necessary for returning the group as a Python-object when
  // grouping via Python
  Object* GetComposite(){
    return m_group;
  }

private:
  Object* m_group;
  std::vector<int> m_objectDepths;
  objects_t m_objects;
  bool m_select;
};

class UngroupObjectsCommand : public Command {
public:
  UngroupObjectsCommand(const objects_t& objects, const select_added& select)
    : Command(CommandType::OBJECT),
      m_objects(objects),
      m_select(select.Get())
  {}

  void Do(CommandContext& context) override{
    // Initialize object depths for Undo on the first run
    if (m_objectDepths.empty()) {
      for (const Object* obj : m_objects){
        m_objectDepths.push_back(context.GetObjectZ(obj));
      }
    }

    // Traverse and ungroup all groups in the command
    for (Object* group : m_objects){
      int numContained = group->GetObjectCount();
      const int depth = context.GetObjectZ(group);
      // Add the contained objects
      for (int j = 0; j != numContained; j++){
        Object* contained = group->GetObject(j);
        context.Add(contained, depth + j, select_added(m_select),
          deselect_old(false)); // Fixme: Causes loads of refresh-es for no good reason
      }
      // Remove the disbanded group
      context.Remove(group);
    }
  }

  utf8_string Name() const override{
    return "Ungroup Objects";
  }

  void Undo(CommandContext& context) override{
    for (auto group : enumerate(m_objects)){
      int numContained = (*group)->GetObjectCount();
      if (numContained == 0){
        continue;
      }
      for (int j = 0; j != numContained; j++){
        context.Remove(group.item->GetObject(j));
      }
      context.Add(group.item, m_objectDepths[to_size_t(group.num)],
        select_added(m_select), deselect_old(true));
    }
  }
private:
  objects_t m_objects;
  std::vector<int> m_objectDepths;
  bool m_select;
};

static bool all_are_groups(const objects_t& objects){
  for (const Object* obj : objects){
    if (obj->GetObjectCount() == 0){
      return false;
    }
  }
  return true;
}

std::pair<Command*, Object*> group_objects_command(const objects_t& objects,
  const select_added& select)
{
  assert(!objects.empty());
  GroupObjectsCommand* cmd = new GroupObjectsCommand(objects, select);
  return std::make_pair(cmd, cmd->GetComposite());
}

Command* ungroup_objects_command(const objects_t& objects,
  const select_added& select)
{
  assert(!objects.empty());
  assert(all_are_groups(objects));
  return new UngroupObjectsCommand(objects, select);
}

} // namespace
