#include "TaskInputWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

ftxui::Component MakeTaskInputWindow(AppState &state) {
  using namespace ftxui;

  auto input = Input({
      .content = &state.draftTask,
      .placeholder = "Enter task title...",
      .multiline = false,
  });

  auto renderer = Renderer(input, [input] {
    return window(text("New task"), vbox({
                                        input->Render(),
                                        separator(),
                                        text("Enter: add  Esc: cancel"),
                                    })) |
           size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape) {
      state.draftTask.clear();
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::Return) {
      AddTask(state, state.draftTask);
      state.draftTask.clear();
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    return false;
  });
}
