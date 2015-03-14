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

#include <cassert>
#include "tasks/null-task.hh"
#include "tasks/task.hh"
#include "util/optional.hh"

namespace faint{

Task* Task::DefaultTask() const{
  // The null-pointer tells the TaskWrapper to set the default-task or
  // request a tool change if no task is available.
  return nullptr;
}

Optional<faint::SelectionContext&> Task::SelectionContext() {
  return const_cast<const Task*>(this)->SelectionContext().Visit(
    [](const faint::SelectionContext& c){
      return Optional<faint::SelectionContext&>(
        const_cast<faint::SelectionContext&>(c));
    },
    [](){
      return Optional<faint::SelectionContext&>();
    });
}

TaskWrapper::TaskWrapper(default_task defaultTask, initial_task initialTask)
  : m_defaultTask(nullptr),
    m_task(nullptr)
{
  Reset(defaultTask, initialTask);
}

TaskWrapper::~TaskWrapper(){
  Clear();
}

void TaskWrapper::Clear(){
  if (m_defaultTask != m_task){
    delete m_task;
    m_task = nullptr;
  }
  delete m_defaultTask;
  m_defaultTask = nullptr;
}

void TaskWrapper::Reset(default_task defaultTask, initial_task initialTask){
  Clear();
  m_defaultTask = defaultTask.Get();
  m_task = initialTask.Get();
  if (m_task == nullptr){
    m_task = defaultTask.Get();
  }
  assert(m_task != nullptr);
  m_task->Activate();
}

Task* TaskWrapper::operator->(){
  assert(m_task != nullptr);
  return m_task;
}

const Task* TaskWrapper::operator->() const{
  assert(m_task != nullptr);
  return m_task;
}

void TaskWrapper::Push(){
  Task* newTask = m_task->GetNewTask();
  assert(newTask != nullptr);
  assert(newTask != m_task);
  m_pushed.push_back(m_task);
  m_task = newTask;
  m_task->Activate();
}

bool TaskWrapper::Switch(){
  Task* newTask = m_task->GetNewTask();
  assert(newTask != m_task);
  if (m_task != m_defaultTask){
    delete m_task;
  }

  if (newTask == nullptr){
    if (!m_pushed.empty()){
      m_task = m_pushed.back();
      m_pushed.pop_back();
      return true;
    }
  }
  else {
    assert(m_pushed.empty());
  }
  m_task = (newTask == nullptr ? m_defaultTask : newTask);
  if (m_task != nullptr){
    m_task->Activate();
    return true;
  }

  // The new task and the default task are both 0, no task left.
  m_task = null_task();
  return false;
}

} // namespace
