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

} // namespace

void RefreshTaskLabels(AppState &state) {
  state.taskLabels.clear();
  state.taskLabels.reserve(state.tasks.size());

  for (const Task &task : state.tasks) {
    state.taskLabels.push_back(task.title);
  }
}

void RefreshTagLabels(AppState &state) {
  state.tagLabels.clear();
  state.tagLabels.reserve(state.tags.size());

  const Task *selectedTask =
      HasSelectedTask(state)
          ? &state.tasks[static_cast<size_t>(state.selectedTask)]
          : nullptr;

  for (const Tag &tag : state.tags) {
    const bool checked =
        selectedTask != nullptr && TaskHasTag(*selectedTask, tag.id);
    state.tagLabels.push_back(std::string(checked ? "[x] " : "[ ] ") +
                              tag.name);
  }
}

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
  RefreshTaskLabels(state);
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

  RefreshTaskLabels(state);
  SaveTasks(state);
}

void DeleteSelectedTask(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  state.tasks.erase(state.tasks.begin() + state.selectedTask);
  ClampSelectedTask(state);

  RefreshTaskLabels(state);
  SaveTasks(state);
}

void ToggleTaskDeprecated(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  task.status = TaskStatus::Deprecated;
  task.statusChangedAt = std::chrono::system_clock::now();

  RefreshTaskLabels(state);
  SaveTasks(state);
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
  RefreshTagLabels(state);
  SaveTasks(state);
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
  RefreshTagLabels(state);
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

  RefreshTagLabels(state);
  SaveTasks(state);
}
