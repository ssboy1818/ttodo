#include "AppActions.h"

#include <algorithm>
#include <chrono>
#include <cctype>
#include <utility>

#include "TaskStorage.h"

namespace {

std::string Trim(std::string value) {
  auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };

  value.erase(value.begin(),
              std::find_if(value.begin(), value.end(),
                           [&](unsigned char c) { return !isSpace(c); }));
  value.erase(std::find_if(value.rbegin(), value.rend(),
                           [&](unsigned char c) { return !isSpace(c); })
                  .base(),
              value.end());
  return value;
}

int TaskStatusSortRank(TaskStatus status) { return static_cast<int>(status); }

void RestoreSelectedTask(AppState &state, int selectedTaskId) {
  for (size_t index = 0; index < state.tasks.size(); ++index) {
    if (state.tasks[index].id == selectedTaskId) {
      state.selectedTask = static_cast<int>(index);
      return;
    }
  }

  ClampSelectedTask(state);
}

} // namespace

void AddTask(AppState &state, std::string title) {
  title = Trim(std::move(title));
  if (title.empty()) {
    return;
  }

  state.tasks.push_back({
      .title = std::move(title),
      .id = state.nextTaskId++,
      .status = TaskStatus::Created,
      .createdAt = std::chrono::system_clock::now(),
  });
  state.selectedTask = static_cast<int>(state.tasks.size()) - 1;
  if (state.showGroupedTasks) {
    SortTasksByStatus(state);
  }
  SaveTasks(state);
}

void ChangeTaskStatus(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  switch (task.status) {
  case TaskStatus::Created:
    task.statusChangedAt = std::chrono::system_clock::now();
    task.status = TaskStatus::InProgress;
    break;
  case TaskStatus::InProgress:
    task.statusChangedAt = std::chrono::system_clock::now();
    task.status = TaskStatus::Completed;
    break;
  case TaskStatus::Completed:
  case TaskStatus::Deprecated:
    task.status = TaskStatus::Created;
    task.statusChangedAt.reset();
    break;
  }

  if (state.showGroupedTasks) {
    SortTasksByStatus(state);
  }

  SaveTasks(state);
}

void DeleteSelectedTask(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  state.tasks.erase(state.tasks.begin() + state.selectedTask);
  ClampSelectedTask(state);
  SaveTasks(state);
}

void ToggleTaskDeprecated(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  task.status = TaskStatus::Deprecated;
  task.statusChangedAt = std::chrono::system_clock::now();

  if (state.showGroupedTasks) {
    SortTasksByStatus(state);
  }

  SaveTasks(state);
}

void SortTasksByStatus(AppState &state) {
  const int selectedTaskId =
      HasSelectedTask(state)
          ? state.tasks[static_cast<size_t>(state.selectedTask)].id
          : 0;

  std::ranges::stable_sort(state.tasks, [](const Task &left,
                                           const Task &right) {
    return TaskStatusSortRank(left.status) < TaskStatusSortRank(right.status);
  });

  RestoreSelectedTask(state, selectedTaskId);
}

void AddTag(AppState &state, std::string name) {
  name = Trim(std::move(name));
  if (name.empty()) {
    return;
  }

  state.tags.push_back({
      .name = std::move(name),
      .id = state.nextTagId++,
  });
  state.selectedTag = static_cast<int>(state.tags.size()) - 1;
  SaveTasks(state);
}

void MoveSelectedTag(AppState &state, int delta) {
  if (state.tags.empty()) {
    state.selectedTag = 0;
    return;
  }

  state.selectedTag =
      std::clamp(state.selectedTag + delta, 0,
                 static_cast<int>(state.tags.size()) - 1);
}

void DeleteSelectedTag(AppState &state) {
  if (state.tags.empty()) {
    state.selectedTag = 0;
    return;
  }

  ClampSelectedTag(state);

  const int tagId = state.tags[static_cast<size_t>(state.selectedTag)].id;
  state.tags.erase(state.tags.begin() + state.selectedTag);

  for (Task &task : state.tasks) {
    std::erase(task.tagIds, tagId);
  }

  ClampSelectedTag(state);
  SaveTasks(state);
}

void ToggleSelectedTaskTag(AppState &state) {
  if (!HasSelectedTask(state) || state.tags.empty()) {
    return;
  }

  ClampSelectedTag(state);

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];
  const int tagId = state.tags[static_cast<size_t>(state.selectedTag)].id;
  if (!TaskHasTag(task, tagId)) {
    task.tagIds.push_back(tagId);
  } else {
    auto tag = std::ranges::find(task.tagIds, tagId);
    task.tagIds.erase(tag);
  }

  SaveTasks(state);
}
