#include "HelpWindow.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeHelpWindow(AppState &state) {
  using namespace ftxui;

  return Renderer([&state] {
    return window(text("Help"),
                  vbox({
                      hbox({text("n") | bold, text("       new task")}),
                      hbox({text("g") | bold, text("       group by status")}),
                      hbox({text("Space") | bold, text("   next status")}),
                      hbox({text("d") | bold, text("       deprecated")}),
                      hbox({text("Ctrl+D") | bold, text("  delete task")}),
                      hbox({text("Up/Down") | bold, text(" select task")}),
                      hbox({text("q") | bold, text("       quit")}),
                      separator(),
                      hbox({text("h") | bold, text(" / "), text("Esc") | bold,
                            text(" close help")}),
                  })) |
           size(WIDTH, GREATER_THAN, 34) | size(WIDTH, LESS_THAN, 60);
  });
}
