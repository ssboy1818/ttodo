#include "HelpWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeHelpWindow(AppState &state) {
  using namespace ftxui;

  auto renderer = Renderer([&state](bool) {
    return window(text("Help"),
                  vbox({
                      hbox({text("n") | bold, text("       new task")}),
                      hbox({text("t") | bold, text("       new tag")}),
                      hbox({text("Enter") | bold, text("   manage task tags")}),
                      hbox({text("g") | bold, text("       group by status")}),
                      hbox({text("Space") | bold, text("   next status")}),
                      hbox({text("d") | bold, text("       deprecated")}),
                      hbox({text("Ctrl+D") | bold, text("  delete task/tag")}),
                      hbox({text("Up/Down") | bold, text(" select task")}),
                      hbox({text("q") | bold, text("       quit")}),
                      separator(),
                      hbox({text("h") | bold, text(" / "), text("Esc") | bold,
                            text(" close help")}),
                  })) |
           size(WIDTH, GREATER_THAN, 34) | size(WIDTH, LESS_THAN, 60);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape || event == Event::Character('h')) {
      state.showHelp = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    return true;
  });
}
