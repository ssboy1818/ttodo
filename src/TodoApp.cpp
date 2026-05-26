#include "TodoApp.h"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <memory>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppState.h"
#include "HelpBar.h"
#include "TaskDetails.h"
#include "TaskInputWindow.h"
#include "TaskList.h"

namespace {

std::string Trim(std::string value) {
  auto isSpace = [](unsigned char c) { return std::isspace(c) != 0; };

  value.erase(value.begin(),
              std::find_if(value.begin(), value.end(),
                           [&](unsigned char c) { return !isSpace(c); }));
  value.erase(std::find_if(value.rbegin(), value.rend(),
                           [&](unsigned char c) { return !isSpace(c); })
                  .base(),
              value.end());
  return value;
}

void ChangeTaskStatus(AppState &state) {
  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  switch (task.status) {
    case TaskStatus::Created:
      task.statusChangedAt = std::chrono::system_clock::now();
      task.status = TaskStatus::InProgress;
      break;
    case TaskStatus::InProgress:
      task.statusChangedAt = std::chrono::system_clock::now();
      task.status = TaskStatus::Completed;
      break;
    case TaskStatus::Completed:
    case TaskStatus::Deprecated:
      task.status = TaskStatus::Created;
      task.statusChangedAt.reset();
      break;
  }
}

void ToggleTaskDeprecated(AppState &state) {
  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  task.status = TaskStatus::Deprecated;
  task.statusChangedAt = std::chrono::system_clock::now();
}

} // namespace

ftxui::Component MakeTodoApp(ftxui::Closure quit) {
  using namespace ftxui;

  auto state = std::make_shared<AppState>();

  auto taskList = MakeTaskList(*state);
  auto taskDetails = MakeTaskDetails(*state);
  auto inputWindow = MakeTaskInputWindow(*state);
  auto helpBar = MakeHelpBar(*state);

  auto eventTarget = Container::Tab(
      {
          taskList,
          inputWindow,
      },
      &state->activeComponent);

  auto submitTask = [state] {
    std::string title = Trim(state->draftTask);
    if (!title.empty()) {
      state->tasks.push_back({
          .title = title,
          .status = TaskStatus::Created,
          .createdAt = std::chrono::system_clock::now(),
      });
      // state->selectedTask = static_cast<int>(state->tasks.size()) - 1;
    }
    state->draftTask.clear();
    state->showInput = false;
  };

  auto cancelInput = [state] {
    state->draftTask.clear();
    state->showInput = false;
  };

  auto renderer = Renderer(eventTarget, [=] {
    Element content = hbox({
        taskList->Render() | flex,
        taskDetails->Render(),
    });

    Element mainWindow = vbox({
        content | flex,
        state->showHelp ? helpBar->Render() : emptyElement(),
    });

    if (!state->showInput) {
      return mainWindow;
    }

    return dbox({
        mainWindow | dim,
        inputWindow->Render() | clear_under | center | vcenter,
    });
  });

  return CatchEvent(renderer, [=](Event event) {
    state->activeComponent = state->showInput ? 1 : 0;

    if (state->showInput) {
      if (event == Event::Escape) {
        cancelInput();
        state->activeComponent = 0;
        return true;
      }
      if (event == Event::Return) {
        submitTask();
        state->activeComponent = 0;
        return true;
      }
      return false;
    }

    if (event == Event::Character('n')) {
      state->draftTask.clear();
      state->showInput = true;
      state->activeComponent = 1;
      return true;
    }
    if (event == Event::Character('h')) {
      state->showHelp = !state->showHelp;
      return true;
    }
    if (event == Event::Character('g')) {
      state->showGroupedTasks = !state->showGroupedTasks;
      return true;
    }
    if (event == Event::Return) {
      ChangeTaskStatus(*state);
      return true;
    }
    if (event == Event::Character('d')) {
      ToggleTaskDeprecated(*state);
      return true;
    }
    if (event == Event::Character('q')) {
      quit();
      return true;
    }

    return false;
  });
}
