#include "TagInputWindow.h"

#include <cstddef>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

namespace {

constexpr int kTagColumns = 3;
constexpr int kVisibleTagRows = 5;
constexpr int kTagCellWidth = 20;

ftxui::Element TagListView(AppState &state) {
  using namespace ftxui;

  if (state.tags.empty()) {
    return text("No tags created") | dim | size(HEIGHT, EQUAL, kVisibleTagRows);
  }

  ClampSelectedTag(state);

  Elements rows;
  const int rowCount =
      (static_cast<int>(state.tags.size()) + kTagColumns - 1) / kTagColumns;
  rows.reserve(static_cast<size_t>(rowCount));

  for (int row = 0; row < rowCount; ++row) {
    Elements cells;
    cells.reserve(kTagColumns);

    for (int column = 0; column < kTagColumns; ++column) {
      const int tagIndex = row * kTagColumns + column;
      if (tagIndex >= static_cast<int>(state.tags.size())) {
        cells.push_back(text("") | size(WIDTH, EQUAL, kTagCellWidth));
        continue;
      }

      const bool selected = state.selectedTag == tagIndex;
      Element cell =
          text(std::string(selected ? "> " : "  ") +
               state.tags[static_cast<size_t>(tagIndex)].name) |
          size(WIDTH, EQUAL, kTagCellWidth);
      if (selected) {
        cell |= inverted;
        cell |= bold;
        cell |= focus;
      }
      cells.push_back(cell);
    }

    rows.push_back(hbox(std::move(cells)));
  }

  return vbox(std::move(rows)) | vscroll_indicator | frame |
         size(HEIGHT, EQUAL, kVisibleTagRows);
}

} // namespace

ftxui::Component MakeTagInputWindow(AppState &state) {
  using namespace ftxui;

  auto input = Input({
      .content = &state.draftTag,
      .placeholder = "Enter tag name...",
      .multiline = false,
  });

  auto renderer = Renderer(input, [&state, input] {
    return window(text("Tags"), vbox({
                                  TagListView(state),
                                  separator(),
                                  input->Render(),
                                  separator(),
                                  text("Enter: add  Ctrl+D: delete  Esc: close"),
                              })) |
           size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
  });

  return CatchEvent(renderer, [&state](Event event) {
    if (event == Event::Escape) {
      state.draftTag.clear();
      state.showTagInput = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::ArrowLeft) {
      MoveSelectedTag(state, -1);
      return true;
    }
    if (event == Event::ArrowRight) {
      MoveSelectedTag(state, 1);
      return true;
    }
    if (event == Event::ArrowUp) {
      MoveSelectedTag(state, -3);
      return true;
    }
    if (event == Event::ArrowDown) {
      MoveSelectedTag(state, 3);
      return true;
    }
    if (event == Event::CtrlD) {
      DeleteSelectedTag(state);
      return true;
    }
    if (event == Event::Return) {
      AddTag(state, state.draftTag);
      state.draftTag.clear();
      state.showTagInput = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    return false;
  });
}
