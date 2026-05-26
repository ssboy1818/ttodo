#pragma once

#include <chrono>
#include <optional>
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
  std::chrono::system_clock::time_point createdAt;
  std::optional<std::chrono::system_clock::time_point> statusChangedAt;
};
