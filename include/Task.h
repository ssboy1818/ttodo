#pragma once

#include <string>

enum class TaskStatus {
  Created,
  InProgress,
  Completed,
  Deprecated,
};

struct Task {
  std::string title;
  TaskStatus status = TaskStatus::Created;
};
