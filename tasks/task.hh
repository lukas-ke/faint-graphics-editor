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

#ifndef FAINT_TASK_HH
#define FAINT_TASK_HH
#include <vector>
#include "app/resource-id.hh"
#include "geo/int-rect.hh"
#include "tools/refresh-info.hh"
#include "util/distinct.hh"
#include "util/optional.hh"
#include "util/pending.hh"

namespace faint{

class Command;
class FaintDC;
class HistoryContext;
class IntRect;
class KeyInfo;
class Overlays;
class Point;
class PosInfo;
class SelectionContext;
class TextContext;
enum class Cursor;
enum class Layer;

enum class TaskResult{
  // Return values for Task-methods
  NONE, // Action had no externally relevant effect
  DRAW, // Task wants to draw
  SETTING_CHANGED, // A setting was changed
  CHANGE, // Task wants to be switched out for return of GetNewTask()
  COMMIT, // Task has a command ready from GetCommand()
  COMMIT_AND_CHANGE, // Task has a command and a new task
  PUSH // Push a new task (revert to this task when popping)
};

class Task{
  // Tasks are used to split Tools into states.
public:
  virtual ~Task() = default;
  virtual void Activate() = 0;
  virtual TaskResult Char(const KeyInfo&) = 0;
  virtual TaskResult DoubleClick(const PosInfo&) = 0;
  virtual void Draw(FaintDC&, Overlays&, const PosInfo&) = 0;
  virtual bool DrawBeforeZoom(Layer) const = 0;
  virtual bool EatsSettings() const = 0;
  virtual Command* GetCommand() = 0;
  virtual Cursor GetCursor(const PosInfo&) const = 0;
  virtual Task* GetNewTask() = 0;
  virtual IntRect GetRefreshRect(const RefreshInfo&) const = 0;
  virtual Optional<const faint::HistoryContext&> HistoryContext() const = 0;
  virtual TaskResult MouseDown(const PosInfo&) = 0;
  virtual TaskResult MouseUp(const PosInfo&) = 0;
  virtual TaskResult MouseMove(const PosInfo&) = 0;
  virtual TaskResult Preempt(const PosInfo&) = 0;
  virtual bool RefreshOnMouseOut() const = 0;
  Optional<faint::SelectionContext&> SelectionContext();
  virtual Optional<const faint::SelectionContext&> SelectionContext() const = 0;
  virtual void SelectionChange() = 0;
  virtual void SetLayer(Layer) = 0;
  virtual Optional<const faint::TextContext&> TextContext() const = 0;
  virtual void UpdateSettings() = 0;
protected:
  Task* DefaultTask() const;
};

class TaskWrapper;
using default_task = Distinct<Task*, TaskWrapper, 1>;
using initial_task = Distinct<Task*, TaskWrapper, 2>;

class TaskWrapper{
  // Contains an active task and a default task. The active task is
  // accesible via operator->.
  //
  // The Switch() function changes to a new task retrieved from the
  // current task using Task::GetNewTask. If the new task is null, the
  // default task will be activated. If the default task is also null,
  // a benign NullTask will be activated, and Switch will return
  // false.
  //
  // Releases the task memory on destruction.
public:
  explicit TaskWrapper(default_task, initial_task=initial_task(nullptr));
  void Reset(default_task, initial_task initial=initial_task(nullptr));
  ~TaskWrapper();
  bool Switch();
  void Push();
  Task* operator->();
  const Task* operator->() const;

  TaskWrapper(const TaskWrapper&) = delete;
  TaskWrapper& operator=(const TaskWrapper&) = delete;
private:
  void Clear();
  Task* m_defaultTask;
  Task* m_task;
  std::vector<Task*> m_pushed;
};

// For tasks to be returned by Task::GetNewTask.
using PendingTask = Pending<Task>;

} // namespace

#endif
