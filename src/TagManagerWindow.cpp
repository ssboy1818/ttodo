#include "TagManagerWindow.h"

#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

ftxui::Component MakeTagManagerWindow(AppState &state) {
  using namespace ftxui;

  return Renderer([&state] {
    if (!HasSelectedTask(state)) {
      return window(text("Task tags"), text("No selected task") | dim) |
             size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
    }

    const Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

    if (state.tags.empty()) {
      return window(text("Task tags"), vbox({
                                           text("No tags created") | dim,
                                           separator(),
                                           text("Press t to create a tag"),
                                           text("Esc: close"),
                                       })) |
             size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
    }

    ClampSelectedTag(state);

    Elements rows;
    rows.reserve(state.tags.size());
    for (size_t index = 0; index < state.tags.size(); ++index) {
      const Tag &tag = state.tags[index];
      const bool selected = state.selectedTag == static_cast<int>(index);
      const bool checked = TaskHasTag(task, tag.id);
      Element row = text(std::string(selected ? "> " : "  ") +
                         (checked ? "[x] " : "[ ] ") + tag.name);
      if (selected) {
        row |= inverted;
        row |= bold;
      }
      rows.push_back(row);
    }

    return window(text("Task tags"),
                  vbox({
                      vbox(std::move(rows)) | vscroll_indicator | frame | flex,
                      separator(),
                      text("Space: toggle  Enter/Esc: close") | dim,
                  })) |
           size(WIDTH, GREATER_THAN, 40) | size(WIDTH, LESS_THAN, 70);
  });
}
