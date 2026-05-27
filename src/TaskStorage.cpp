#include "TaskStorage.h"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>
#include <vector>

namespace {

std::filesystem::path StoragePath() {
  const char *xdgDataHome = std::getenv("XDG_DATA_HOME");
  const char *home = std::getenv("HOME");

  std::filesystem::path base =
      xdgDataHome != nullptr && !std::string(xdgDataHome).empty()
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

std::vector<int> ParseTagIds(const std::string &value) {
  std::vector<int> tagIds;
  std::stringstream stream(value);
  std::string tagId;

  while (std::getline(stream, tagId, ',')) {
    if (tagId.empty()) {
      continue;
    }

    try {
      tagIds.push_back(std::stoi(tagId));
    } catch (const std::exception &) {
    }
  }

  return tagIds;
}

std::string JoinTagIds(const std::vector<int> &tagIds) {
  std::string result;

  for (int tagId : tagIds) {
    if (!result.empty()) {
      result += ",";
    }
    result += std::to_string(tagId);
  }

  return result;
}

std::optional<Task> ParseTaskFields(const std::vector<std::string> &fields,
                                    size_t offset) {
  try {
    const long long statusChangedAt = std::stoll(fields[offset + 4]);

    Task task{
        .title = UnescapeField(fields[offset + 1]),
        .id = std::stoi(fields[offset]),
        .status = ParseStatus(fields[offset + 2]),
        .createdAt = FromEpochSeconds(std::stoll(fields[offset + 3])),
        .statusChangedAt =
            statusChangedAt > 0
                ? std::make_optional(FromEpochSeconds(statusChangedAt))
                : std::nullopt,
        .tagIds = fields.size() > offset + 5 ? ParseTagIds(fields[offset + 5])
                                             : std::vector<int>{},
    };

    return task;
  } catch (const std::exception &) {
    return std::nullopt;
  }
}

std::optional<Task> ParseTask(const std::vector<std::string> &fields) {
  if (fields.size() == 5) {
    return ParseTaskFields(fields, 0);
  }
  if ((fields.size() == 6 || fields.size() == 7) && fields[0] == "TASK") {
    return ParseTaskFields(fields, 1);
  }

  return std::nullopt;
}

std::optional<Tag> ParseTag(const std::vector<std::string> &fields) {
  if (fields.size() != 3 || fields[0] != "TAG") {
    return std::nullopt;
  }

  try {
    return Tag{
        .name = UnescapeField(fields[2]),
        .id = std::stoi(fields[1]),
    };
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
  state.tags.clear();

  std::string line;
  while (std::getline(input, line)) {
    const std::vector<std::string> fields = SplitTabs(line);

    std::optional<Tag> tag = ParseTag(fields);
    if (tag.has_value()) {
      state.tags.push_back(*tag);
      continue;
    }

    std::optional<Task> task = ParseTask(fields);
    if (task.has_value()) {
      state.tasks.push_back(*task);
    }
  }

  int maxTaskId = 0;
  for (const Task &task : state.tasks) {
    maxTaskId = std::max(maxTaskId, task.id);
  }

  int maxTagId = 0;
  for (const Tag &tag : state.tags) {
    maxTagId = std::max(maxTagId, tag.id);
  }

  state.nextTaskId = maxTaskId + 1;
  state.nextTagId = maxTagId + 1;
  state.selectedTask = state.tasks.empty() ? 0 : 0;
}

void SaveTasks(const AppState &state) {
  const std::filesystem::path path = StoragePath();
  std::filesystem::create_directories(path.parent_path());

  std::ofstream output(path, std::ios::trunc);
  if (!output.is_open()) {
    return;
  }

  for (const Tag &tag : state.tags) {
    output << "TAG" << '\t' << tag.id << '\t' << EscapeField(tag.name) << '\n';
  }

  for (const Task &task : state.tasks) {
    output << "TASK" << '\t' << task.id << '\t' << EscapeField(task.title)
           << '\t' << static_cast<int>(task.status) << '\t'
           << ToEpochSeconds(task.createdAt) << '\t'
           << (task.statusChangedAt.has_value()
                   ? ToEpochSeconds(*task.statusChangedAt)
                   : 0)
           << '\t' << JoinTagIds(task.tagIds) << '\n';
  }
}
