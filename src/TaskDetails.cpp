#include "TaskDetails.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <optional>
#include <sstream>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace {

std::string TaskStatusName(TaskStatus status) {
  switch (status) {
  case TaskStatus::Created:
    return "created";
  case TaskStatus::InProgress:
    return "pending";
  case TaskStatus::Completed:
    return "completed";
  case TaskStatus::Deprecated:
    return "deprecated";
  }

  return "unknown";
}

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

ftxui::Element DetailRow(const std::string &label, const std::string &value) {
  using namespace ftxui;

  return hbox({
      text(label) | dim,
      paragraph(value),
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
      hbox({text("Created") | dim, filler(), text(CountText(stats.created))}),
      hbox({text("In progress") | dim, filler(),
            text(CountText(stats.inProgress))}),
      hbox({text("Completed") | dim, filler(),
            text(CountText(stats.completed))}),
      hbox({text("Deprecated") | dim, filler(),
            text(CountText(stats.deprecated))}),
      separator(),
      hbox({text("Progress") | dim, filler(),
            text(std::to_string(static_cast<int>(progress * 100.0F)) + "%")}),
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
