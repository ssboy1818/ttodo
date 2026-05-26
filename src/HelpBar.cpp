#include "HelpBar.h"

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeHelpBar(AppState &state) {
  using namespace ftxui;

  return Renderer([&] {
    if (!state.showHelp) {
      return emptyElement();
    }

    return hbox({
               text(" n ") | bold,       text("new"),         separatorEmpty(),
               text(" h ") | bold,       text("hide help"),   separatorEmpty(),
               text(" g ") | bold,       text("group"),       separatorEmpty(),
               text(" Space ") | bold,   text("next status"), separatorEmpty(),
               text(" d ") | bold,       text("deprecated"),  separatorEmpty(),
               text(" q ") | bold,       text("quit"),        separatorEmpty(),
               text(" Up/Down ") | bold, text("select"),
           }) |
           border;
  });
}
