#include "TaskList.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>

#include "AppActions.h"
#include "MenuStyles.h"

namespace {

ftxui::Element TaskMenuEntry(const AppState &state,
                             const ftxui::EntryState &entryState) {
  using namespace ftxui;

  const TaskStatus status = state.tasks[entryState.index].status;

  Element entry = hbox({
      text(entryState.active ? "> " : "  "),
      text(TaskStatusIcon(status)) | color(TaskStatusColor(status)),
      text(" " + entryState.label),
  });

  if (entryState.active) {
    entry |= bgcolor(Color::GrayDark) | color(Color::Black) | bold;
  }
  if (!entryState.active) {
    entry |= dim;
  }

  return entry;
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
                  [&state](const EntryState &entryState) {
                    return TaskMenuEntry(state, entryState);
                  },
          },
  });

  RefreshTaskLabels(state);

  return Renderer(menu, [&state, menu] {
    Element content = vbox({
        filler(),
        text("No tasks yet") | center | dim,
        text("Press n to add a task") | center | dim,
        filler(),
    });

    if (!state.tasks.empty()) {
      content = menu->Render() | vscroll_indicator | frame | flex;
    }

    return window(text("ttodo"), content) | flex;
  });
}
