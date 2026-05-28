#include "MenuStyles.h"

#include <ftxui/screen/color.hpp>

ftxui::Color TaskStatusColor(TaskStatus status) {
  switch (status) {
    case TaskStatus::Created:
      return ftxui::Color::White;
    case TaskStatus::InProgress:
      return ftxui::Color::Orange1;
    case TaskStatus::Completed:
      return ftxui::Color::Green;
    case TaskStatus::Deprecated:
      return ftxui::Color::DarkRed;
  }

  return ftxui::Color::White;
}

ftxui::Element TagMenuEntry(const ftxui::EntryState &entryState) {
  using namespace ftxui;

  Element entry = text((entryState.active ? "> " : "  ") + entryState.label);
  if (entryState.active) {
    entry |= bgcolor(Color::GrayDark) | color(Color::Black) | bold;
  }
  if (!entryState.active) {
    entry |= dim;
  }

  return entry;
}

ftxui::Element TextInputEntry(ftxui::InputState inputState) {
  using namespace ftxui;

  if (inputState.is_placeholder) {
    inputState.element |= dim;
  }
  if (inputState.focused) {
    inputState.element |= bgcolor(Color::GrayDark) | color(Color::Black);
  } else if (inputState.hovered) {
    inputState.element |= underlined;
  }

  return inputState.element;
}
