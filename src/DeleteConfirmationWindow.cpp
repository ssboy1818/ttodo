#include "DeleteConfirmationWindow.h"

#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

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

  auto renderer = Renderer([&state](bool) {
    return window(text("Delete task"),
                  vbox({
                      paragraph("Delete \"" + SelectedTaskTitle(state) + "\"?"),
                      separator(),
                      text("Enter: delete  Esc: cancel") | dim,
                  })) |
           size(WIDTH, GREATER_THAN, 40) | size(WIDTH, LESS_THAN, 70);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape) {
      state.showDeleteConfirmation = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::Return) {
      DeleteSelectedTask(state);
      state.showDeleteConfirmation = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    return false;
  });
}
