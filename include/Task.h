#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <vector>

enum class TaskStatus {
  Created,
  InProgress,
  Completed,
  Deprecated,
};

struct Task {
  std::string title;
  int id = 0;
  TaskStatus status = TaskStatus::Created;
  std::chrono::system_clock::time_point createdAt;
  std::optional<std::chrono::system_clock::time_point> statusChangedAt;
  std::vector<int> tagIds;
};

struct Tag {
  std::string name;
  int id = 0;
};
