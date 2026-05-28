#include "TaskDetails.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <ranges>
#include <sstream>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include "MenuStyles.h"

namespace {

std::string FormatTimestamp(std::chrono::system_clock::time_point timestamp) {
  const std::time_t rawTime = std::chrono::system_clock::to_time_t(timestamp);
  std::tm localTime{};
  localtime_r(&rawTime, &localTime);

  std::ostringstream output;
  output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
  return output.str();
}

struct TaskStats {
  int total = 0;
  int created = 0;
  int inProgress = 0;
  int completed = 0;
  int deprecated = 0;
};

TaskStats CalculateTaskStats(const AppState &state) {
  TaskStats stats;
  stats.total = static_cast<int>(state.tasks.size());

  for (const Task &task : state.tasks) {
    switch (task.status) {
    case TaskStatus::Created:
      ++stats.created;
      break;
    case TaskStatus::InProgress:
      ++stats.inProgress;
      break;
    case TaskStatus::Completed:
      ++stats.completed;
      break;
    case TaskStatus::Deprecated:
      ++stats.deprecated;
      break;
    }
  }

  return stats;
}

std::string CountText(int count) { return std::to_string(count); }

std::string TaskTagsText(const AppState &state, const Task &task) {
  std::string tags;

  for (int tagId : task.tagIds) {
    auto tag = std::ranges::find_if(state.tags, [tagId](const Tag &candidate) {
      return candidate.id == tagId;
    });
    if (tag == state.tags.end()) {
      continue;
    }

    if (!tags.empty()) {
      tags += ", ";
    }
    tags += tag->name;
  }

  return tags.empty() ? "none" : tags;
}

ftxui::Element DetailRow(const std::string &label, const std::string &value) {
  using namespace ftxui;

  return hbox({
      text(label) | dim,
      paragraph(value),
  });
}

ftxui::Element StatRow(const std::string &label, int count,
                       ftxui::Color rowColor) {
  using namespace ftxui;

  return hbox({
      text(label) | color(rowColor),
      filler(),
      text(CountText(count)) | color(rowColor) | bold,
  });
}

ftxui::Element StatsView(const TaskStats &stats) {
  using namespace ftxui;

  const float progress = stats.total == 0
                             ? 0.0F
                             : static_cast<float>(stats.completed) /
                                   static_cast<float>(stats.total);

  return vbox({
      hbox({text("Total") | dim, filler(), text(CountText(stats.total))}),
      StatRow("Created", stats.created, TaskStatusColor(TaskStatus::Created)),
      StatRow("In progress", stats.inProgress,
              TaskStatusColor(TaskStatus::InProgress)),
      StatRow("Completed", stats.completed,
              TaskStatusColor(TaskStatus::Completed)),
      StatRow("Deprecated", stats.deprecated,
              TaskStatusColor(TaskStatus::Deprecated)),
      separator(),
      hbox({text("Progress") | dim, filler(),
            text(std::to_string(static_cast<int>(progress * 100.0F)) + "%") | bold}),
      gauge(progress),
  });
}

} // namespace

ftxui::Component MakeTaskDetails(AppState &state) {
  using namespace ftxui;

  return Renderer([&state] {
    const TaskStats stats = CalculateTaskStats(state);

    if (state.tasks.empty()) {
      return window(text("statistic"),
                    vbox({
                        StatsView(stats),
                        separator(),
                        text("No selected task") | center | dim,
                        filler(),
                    })) |
             size(WIDTH, EQUAL, 36);
    }

    const int selectedTask = std::clamp(
        state.selectedTask, 0, static_cast<int>(state.tasks.size()) - 1);
    const Task &task = state.tasks[static_cast<size_t>(selectedTask)];

    Elements rows{
        StatsView(stats),
        separator(),
        DetailRow("title: ", task.title),
        DetailRow("status: ", TaskStatusName(task.status)),
        DetailRow("tags: ", TaskTagsText(state, task)),
        DetailRow("created at: ", FormatTimestamp(task.createdAt)),
    };

    if (task.status != TaskStatus::Created &&
        task.statusChangedAt.has_value()) {
      rows.push_back(DetailRow(TaskStatusName(task.status) + " at: ",
                               FormatTimestamp(*task.statusChangedAt)));
    }

    rows.push_back(filler());

    return window(text("statistic"), vbox(std::move(rows))) |
           size(WIDTH, EQUAL, 36);
  });
}
