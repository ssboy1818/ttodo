#pragma once

#include <ftxui/component/component_options.hpp>
#include <ftxui/dom/elements.hpp>

#include "Task.h"

ftxui::Color TaskStatusColor(TaskStatus status);
ftxui::Element TagMenuEntry(const ftxui::EntryState &entryState);
ftxui::Element TextInputEntry(ftxui::InputState inputState);
