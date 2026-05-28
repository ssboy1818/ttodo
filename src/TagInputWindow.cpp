#include "TagInputWindow.h"

#include <memory>
#include <string>
#include <vector>

#include <ftxui/component/component.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"

namespace {

constexpr int kInputPane = 0;
constexpr int kMenuPane = 1;
constexpr int kVisibleTagRows = 8;

void RefreshTagLabels(const AppState &state,
                      std::vector<std::string> &tagLabels) {
  tagLabels.clear();
  tagLabels.reserve(state.tags.size());

  for (const Tag &tag : state.tags) {
    tagLabels.push_back(tag.name);
  }
}

} // namespace

ftxui::Component MakeTagInputWindow(AppState &state) {
  using namespace ftxui;

  auto tagLabels = std::make_shared<std::vector<std::string>>();
  auto focusedPane = std::make_shared<int>(kInputPane);

  auto input = Input({
      .content = &state.draftTag,
      .placeholder = "Enter tag name...",
      .multiline = false,
  });

  auto menu = Menu({
      .entries = tagLabels.get(),
      .selected = &state.selectedTag,
  });

  auto content = Container::Vertical(
      {
          input,
          menu,
      },
      focusedPane.get());

  auto renderer = Renderer(content, [&state, tagLabels, input, menu] {
    RefreshTagLabels(state, *tagLabels);
    ClampSelectedTag(state);

    Element tagList = tagLabels->empty()
                          ? text("No tags created") | dim |
                                size(HEIGHT, EQUAL, kVisibleTagRows)
                          : menu->Render() | vscroll_indicator | frame |
                                size(HEIGHT, EQUAL, kVisibleTagRows);

    return window(text("Tags"), vbox({
                                  tagList,
                                  separator(),
                                  input->Render(),
                                  separator(),
                                  text("Tab: focus  Enter: add  Ctrl+D: "
                                       "delete  Esc: close"),
                              })) |
           size(WIDTH, GREATER_THAN, 36) | size(WIDTH, LESS_THAN, 64);
  });

  return CatchEvent(renderer, [&state, focusedPane, input](Event event) {
    if (event == Event::Escape) {
      state.draftTag.clear();
      state.showTagInput = false;
      state.activeComponent = ActiveComponent::TaskList;
      return true;
    }
    if (event == Event::Tab || event == Event::TabReverse) {
      *focusedPane = *focusedPane == kInputPane ? kMenuPane : kInputPane;
      return true;
    }
    if (event == Event::CtrlD) {
      DeleteSelectedTag(state);
      return true;
    }
    if (event == Event::Return && input->Focused()) {
      AddTag(state, state.draftTag);
      state.draftTag.clear();
      return true;
    }
    return false;
  });
}
