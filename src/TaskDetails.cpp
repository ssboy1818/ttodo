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

std::string FormatTimestamp(std::chrono::system_clock::time_point timestamp) {
  const std::time_t rawTime = std::chrono::system_clock::to_time_t(timestamp);
  std::tm localTime{};
  localtime_r(&rawTime, &localTime);

  std::ostringstream output;
  output << std::put_time(&localTime, "%Y-%m-%d %H:%M:%S");
  return output.str();
}

ftxui::Element DetailRow(const std::string &label, const std::string &value) {
  using namespace ftxui;

  return vbox({
      text(label) | dim,
      paragraph(value),
  });
}

} // namespace

ftxui::Component MakeTaskDetails(AppState &state) {
  using namespace ftxui;

  return Renderer([&state] {
    if (state.tasks.empty()) {
      return window(text("Details"),
                    vbox({
                        filler(),
                        text("No selected task") | center | dim,
                        filler(),
                    })) |
             size(WIDTH, EQUAL, 24);
    }

    const int selectedTask = std::clamp(
        state.selectedTask, 0, static_cast<int>(state.tasks.size()) - 1);
    const Task &task = state.tasks[static_cast<size_t>(selectedTask)];

    Elements rows{
        DetailRow("Title", task.title),
        separator(),
        DetailRow("Status", TaskStatusName(task.status)),
        separator(),
        DetailRow("Created at", FormatTimestamp(task.createdAt)),
    };

    if (task.status != TaskStatus::Created &&
        task.statusChangedAt.has_value()) {
      rows.push_back(separator());
      rows.push_back(DetailRow(TaskStatusName(task.status) + " at",
                               FormatTimestamp(*task.statusChangedAt)));
    }

    rows.push_back(filler());

    return window(text("Details"), vbox(std::move(rows))) |
           size(WIDTH, EQUAL, 36);
  });
}
