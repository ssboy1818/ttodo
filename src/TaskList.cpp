#include "TaskList.h"

#include <algorithm>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

namespace {

void RefreshTaskLabels(AppState &state) {
  state.taskLabels.clear();
  state.taskLabels.reserve(state.tasks.size());

  for (const Task &task : state.tasks) {
    state.taskLabels.push_back(TaskStatusIcon(task.status) + " " + task.title);
  }
}

ftxui::Element TaskGroup(const AppState &state, TaskStatus status) {
  using namespace ftxui;

  Elements rows;

  for (size_t index = 0; index < state.tasks.size(); ++index) {
    const Task &task = state.tasks[index];
    if (task.status != status) {
      continue;
    }

    const bool selected = state.selectedTask == static_cast<int>(index);
    Element row = text((selected ? "> " : "  ") + TaskStatusIcon(status) + " " +
                       task.title);
    if (selected) {
      row |= inverted;
      row |= bold;
    }
    rows.push_back(row);
  }

  if (rows.empty()) {
    rows.push_back(text("No tasks") | dim);
  }

  return window(text(TaskStatusName(status)),
                vbox(std::move(rows)) | vscroll_indicator | frame) |
         flex;
}

ftxui::Element GroupedTaskView(const AppState &state) {
  using namespace ftxui;

  return vbox({
             hbox({
                 TaskGroup(state, TaskStatus::Created),
                 TaskGroup(state, TaskStatus::InProgress),
             }) | flex,
             hbox({
                 TaskGroup(state, TaskStatus::Completed),
                 TaskGroup(state, TaskStatus::Deprecated),
             }) | flex,
         }) |
         flex;
}

} // namespace

ftxui::Component MakeTaskList(AppState &state) {
  using namespace ftxui;

  auto menu = Menu({
      .entries = &state.taskLabels,
      .selected = &state.selectedTask,
      .entries_option =
          {
              .transform =
                  [](const EntryState &entryState) {
                    Element entry = text((entryState.active ? "> " : "  ") +
                                         entryState.label);
                    if (entryState.focused) {
                      entry |= inverted;
                    }
                    if (entryState.active) {
                      entry |= bold;
                    }
                    if (!entryState.focused && !entryState.active) {
                      entry |= dim;
                    }
                    return entry;
                  },
          },
  });

  return Renderer(menu, [&state, menu] {
    RefreshTaskLabels(state);

    state.selectedTask =
        std::clamp(state.selectedTask, 0,
                   std::max(0, static_cast<int>(state.tasks.size()) - 1));

    Element content = vbox({
        filler(),
        text("No tasks yet") | center | dim,
        text("Press n to add a task") | center | dim,
        filler(),
    });

    if (!state.tasks.empty()) {
      content = state.showGroupedTasks
                    ? GroupedTaskView(state)
                    : menu->Render() | vscroll_indicator | frame | flex;
    }

    return window(text("ttodo"), content) | flex;
  });
}
