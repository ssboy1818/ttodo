#pragma once

#include <string>
#include <vector>

#include "Task.h"

struct AppState {
  std::vector<Task> tasks;
  std::vector<std::string> taskLabels;
  int nextTaskId = 1;
  int selectedTask = 0;
  int activeComponent = 0;
  bool showHelp = true;
  bool showInput = false;
  bool showDeleteConfirmation = false;
  bool showGroupedTasks = false;
  std::string draftTask;
};
