#include "Task.h"

#include <algorithm>

std::string TaskStatusIcon(TaskStatus status) {
  switch (status) {
  case TaskStatus::Created:
    return "";
  case TaskStatus::InProgress:
    return "";
  case TaskStatus::Completed:
    return "";
  case TaskStatus::Deprecated:
    return "";
  }

  return "?";
}

std::string TaskStatusName(TaskStatus status) {
  switch (status) {
  case TaskStatus::Created:
    return "Created";
  case TaskStatus::InProgress:
    return "In progress";
  case TaskStatus::Completed:
    return "Completed";
  case TaskStatus::Deprecated:
    return "Deprecated";
  }

  return "Unknown";
}

bool TaskHasTag(const Task &task, int tagId) {
  return std::ranges::find(task.tagIds, tagId) != task.tagIds.end();
}
