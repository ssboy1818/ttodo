#include "TagManagerWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

ftxui::Component MakeTagManagerWindow(AppState &state) {
  using namespace ftxui;

  auto menu = Menu({
      .entries = &state.tagLabels,
      .selected = &state.selectedTag,
  });

  auto renderer = Renderer(menu, [&state, menu] {
    if (!HasSelectedTask(state)) {
      return window(text("Task tags"), text("No selected task") | dim) |
             size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
    }

    if (state.tags.empty()) {
      return window(text("Task tags"), vbox({
                                           text("No tags created") | dim,
                                           separator(),
                                           text("Press t to create a tag"),
                                           text("Esc: close"),
                                       })) |
             size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
    }

    return window(text("Task tags"),
                  vbox({
                      menu->Render() | vscroll_indicator | frame | flex,
                      separator(),
                      text("Space: toggle  Enter/Esc: close") | dim,
                  })) |
           size(WIDTH, GREATER_THAN, 40) | size(WIDTH, LESS_THAN, 70);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape || event == Event::Return) {
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::Character(' ')) {
      ToggleSelectedTaskTag(state);
      return true;
    }
    return false;
  });
}
