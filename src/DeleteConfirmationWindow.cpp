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

std::string SelectedTagName(const AppState &state) {
  if (state.selectedTag < 0 ||
      state.selectedTag >= static_cast<int>(state.tags.size())) {
    return "selected tag";
  }

  return state.tags[static_cast<size_t>(state.selectedTag)].name;
}

std::string DeleteTitle(const AppState &state) {
  return state.deleteTarget == DeleteTarget::Tag ? "Delete tag" : "Delete task";
}

ftxui::Element DeleteMessage(const AppState &state) {
  using namespace ftxui;

  if (state.deleteTarget == DeleteTarget::Tag) {
    return vbox({
        paragraph("Delete tag \"" + SelectedTagName(state) + "\"?"),
        paragraph("This removes it from all tasks.") | dim,
    });
  }

  return paragraph("Delete task \"" + SelectedTaskTitle(state) + "\"?");
}

void CloseDeleteConfirmation(AppState &state) {
  state.showDeleteConfirmation = false;
  state.activeComponent = state.deleteReturnComponent;
}

} // namespace

ftxui::Component MakeDeleteConfirmationWindow(AppState &state) {
  using namespace ftxui;

  auto renderer = Renderer([&state](bool) {
    return window(text(DeleteTitle(state)),
                  vbox({
                      DeleteMessage(state),
                      separator(),
                      text("Enter: delete  Esc: cancel") | dim,
                  })) |
           size(WIDTH, GREATER_THAN, 40) | size(WIDTH, LESS_THAN, 70);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape) {
      CloseDeleteConfirmation(state);
      return true;
    }
    if (event == Event::Return) {
      if (state.deleteTarget == DeleteTarget::Tag) {
        DeleteSelectedTag(state);
      } else {
        DeleteSelectedTask(state);
      }
      CloseDeleteConfirmation(state);
      return true;
    }
    return false;
  });
}
