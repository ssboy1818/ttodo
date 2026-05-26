#include "DeleteConfirmationWindow.h"

#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace {

std::string SelectedTaskTitle(const AppState &state) {
  if (state.selectedTask < 0 ||
      state.selectedTask >= static_cast<int>(state.tasks.size())) {
    return "selected task";
  }

  return state.tasks[static_cast<size_t>(state.selectedTask)].title;
}

} // namespace

ftxui::Component MakeDeleteConfirmationWindow(AppState &state) {
  using namespace ftxui;

  return Renderer([&state] {
    return window(text("Delete task"),
                  vbox({
                      paragraph("Delete \"" + SelectedTaskTitle(state) + "\"?"),
                      separator(),
                      text("Enter: delete  Esc: cancel") | dim,
                  })) |
           size(WIDTH, GREATER_THAN, 40) | size(WIDTH, LESS_THAN, 70);
  });
}
