#pragma once

#include <string>
#include <vector>

#include "Task.h"

struct AppState {
  std::vector<Task> tasks;
  std::vector<Tag> tags;
  std::vector<std::string> taskLabels;
  int nextTaskId = 1;
  int nextTagId = 1;
  int selectedTask = 0;
  int selectedTag = 0;
  int activeComponent = 0;
  bool showHelp = false;
  bool showInput = false;
  bool showTagInput = false;
  bool showTagManager = false;
  bool showDeleteConfirmation = false;
  bool showGroupedTasks = false;
  std::string draftTask;
  std::string draftTag;
};

namespace ActiveComponent {
inline constexpr int TaskList = 0;
inline constexpr int TaskInput = 1;
inline constexpr int TagInput = 2;
inline constexpr int TagManager = 3;
inline constexpr int DeleteConfirmation = 4;
inline constexpr int Help = 5;
} // namespace ActiveComponent

bool HasSelectedTask(const AppState &state);
void ClampSelectedTask(AppState &state);
void ClampSelectedTag(AppState &state);
