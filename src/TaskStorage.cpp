#include "TaskStorage.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path StoragePath() {
  const char *xdgDataHome = std::getenv("XDG_DATA_HOME");
  const char *home = std::getenv("HOME");

  std::filesystem::path base =
      xdgDataHome != nullptr && std::string(xdgDataHome).size() > 0
          ? std::filesystem::path(xdgDataHome)
          : std::filesystem::path(home != nullptr ? home : ".") /
                ".local/share";

  return base / "ttodo" / "tasks.tsv";
}

long long ToEpochSeconds(std::chrono::system_clock::time_point timestamp) {
  return std::chrono::duration_cast<std::chrono::seconds>(
             timestamp.time_since_epoch())
      .count();
}

std::chrono::system_clock::time_point FromEpochSeconds(long long timestamp) {
  return std::chrono::system_clock::time_point{std::chrono::seconds(timestamp)};
}

std::string EscapeField(const std::string &value) {
  std::string escaped;
  escaped.reserve(value.size());

  for (char character : value) {
    switch (character) {
    case '\\':
      escaped += "\\\\";
      break;
    case '\n':
      escaped += "\\n";
      break;
    case '\t':
      escaped += "\\t";
      break;
    default:
      escaped += character;
      break;
    }
  }

  return escaped;
}

std::string UnescapeField(const std::string &value) {
  std::string unescaped;
  unescaped.reserve(value.size());

  bool escaped = false;
  for (char character : value) {
    if (!escaped && character == '\\') {
      escaped = true;
      continue;
    }

    if (escaped) {
      switch (character) {
      case 'n':
        unescaped += '\n';
        break;
      case 't':
        unescaped += '\t';
        break;
      default:
        unescaped += character;
        break;
      }
      escaped = false;
      continue;
    }

    unescaped += character;
  }

  if (escaped) {
    unescaped += '\\';
  }

  return unescaped;
}

std::vector<std::string> SplitTabs(const std::string &line) {
  std::vector<std::string> fields;
  std::stringstream stream(line);
  std::string field;

  while (std::getline(stream, field, '\t')) {
    fields.push_back(field);
  }

  return fields;
}

TaskStatus ParseStatus(const std::string &value) {
  const int status = std::stoi(value);
  if (status < static_cast<int>(TaskStatus::Created) ||
      status > static_cast<int>(TaskStatus::Deprecated)) {
    return TaskStatus::Created;
  }

  return static_cast<TaskStatus>(status);
}

std::optional<Task> ParseTask(const std::string &line) {
  const std::vector<std::string> fields = SplitTabs(line);
  if (fields.size() != 5) {
    return std::nullopt;
  }

  try {
    const long long statusChangedAt = std::stoll(fields[4]);

    Task task{
        .title = UnescapeField(fields[1]),
        .id = std::stoi(fields[0]),
        .status = ParseStatus(fields[2]),
        .createdAt = FromEpochSeconds(std::stoll(fields[3])),
        .statusChangedAt =
            statusChangedAt > 0
                ? std::make_optional(FromEpochSeconds(statusChangedAt))
                : std::nullopt,
    };

    return task;
  } catch (const std::exception &) {
    return std::nullopt;
  }
}

} // namespace

void LoadTasks(AppState &state) {
  std::ifstream input(StoragePath());
  if (!input.is_open()) {
    return;
  }

  state.tasks.clear();

  std::string line;
  while (std::getline(input, line)) {
    std::optional<Task> task = ParseTask(line);
    if (task.has_value()) {
      state.tasks.push_back(*task);
    }
  }

  int maxTaskId = 0;
  for (const Task &task : state.tasks) {
    maxTaskId = std::max(maxTaskId, task.id);
  }

  state.nextTaskId = maxTaskId + 1;
  state.selectedTask = state.tasks.empty() ? 0 : 0;
}

void SaveTasks(const AppState &state) {
  const std::filesystem::path path = StoragePath();
  std::filesystem::create_directories(path.parent_path());

  std::ofstream output(path, std::ios::trunc);
  if (!output.is_open()) {
    return;
  }

  for (const Task &task : state.tasks) {
    output << task.id << '\t' << EscapeField(task.title) << '\t'
           << static_cast<int>(task.status) << '\t'
           << ToEpochSeconds(task.createdAt) << '\t'
           << (task.statusChangedAt.has_value()
                   ? ToEpochSeconds(*task.statusChangedAt)
                   : 0)
           << '\n';
  }
}
