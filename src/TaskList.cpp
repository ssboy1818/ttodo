#include "TaskList.h"

#include <algorithm>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

namespace {

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

void RefreshTaskLabels(AppState &state) {
  state.taskLabels.clear();
  state.taskLabels.reserve(state.tasks.size());

  for (const Task &task : state.tasks) {
    state.taskLabels.push_back(TaskStatusIcon(task.status) + " " + task.title);
  }
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

    Element content = state.tasks.empty()
                          ? vbox({
                                filler(),
                                text("No tasks yet") | center | dim,
                                text("Press n to add a task") | center | dim,
                                filler(),
                            })
                          : menu->Render() | vscroll_indicator | frame | flex;

    return window(text("Tasks"), content) | flex;
  });
}
