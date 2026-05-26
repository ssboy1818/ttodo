#include "TaskInputWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeTaskInputWindow(AppState &state) {
  using namespace ftxui;

  auto input = Input({
      .content = &state.draftTask,
      .placeholder = "Enter task title...",
      .multiline = false,
  });

  return Renderer(input, [input] {
    return window(text("New task"), vbox({
                                        input->Render(),
                                        separator(),
                                        text("Enter: add  Esc: cancel"),
                                    })) |
           size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
  });
}
