#include "TagInputWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeTagInputWindow(AppState &state) {
  using namespace ftxui;

  auto input = Input({
      .content = &state.draftTag,
      .placeholder = "Enter tag name...",
      .multiline = false,
  });

  return Renderer(input, [input] {
    return window(text("New tag"), vbox({
                                       input->Render(),
                                       separator(),
                                       text("Enter: add  Esc: cancel"),
                                   })) |
           size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
  });
}
