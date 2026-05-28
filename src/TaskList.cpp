#include "TaskList.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

ftxui::Component MakeTaskList(AppState &state) {
  using namespace ftxui;

  auto menu = Menu({
      .entries = &state.taskLabels,
      .selected = &state.selectedTask,
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
