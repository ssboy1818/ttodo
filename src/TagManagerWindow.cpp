#include "TagManagerWindow.h"

#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

ftxui::Component MakeTagManagerWindow(AppState &state) {
  using namespace ftxui;

  auto renderer = Renderer([&state](bool) {
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

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape || event == Event::Return) {
      state.showTagManager = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::Character('t')) {
      state.draftTag.clear();
      state.showTagManager = false;
      state.showTagInput = true;
      state.activeComponent = ActiveComponent::TagInput;
      return true;
    }
    if (event == Event::ArrowUp) {
      MoveSelectedTag(state, -1);
      return true;
    }
    if (event == Event::ArrowDown) {
      MoveSelectedTag(state, 1);
      return true;
    }
    if (event == Event::Character(' ')) {
      ToggleSelectedTaskTag(state);
      return true;
    }
    return true;
  });
}
