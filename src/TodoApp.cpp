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

bool HasSelectedTask(const AppState &state) {
  return state.selectedTask >= 0 &&
         state.selectedTask < static_cast<int>(state.tasks.size());
}

void ToggleSelectedTaskCompletion(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];
  task.status = task.status == TaskStatus::Completed ? TaskStatus::Created
                                                     : TaskStatus::Completed;
}

void ToggleSelectedTaskProgress(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];
  task.status = task.status == TaskStatus::InProgress ? TaskStatus::Created
                                                      : TaskStatus::InProgress;
}

void ToggleSelectedTaskDeprecated(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];
  task.status = task.status == TaskStatus::Deprecated ? TaskStatus::Created
                                                      : TaskStatus::Deprecated;
}

} // namespace

ftxui::Component MakeTodoApp(ftxui::Closure quit) {
  using namespace ftxui;

  auto state = std::make_shared<AppState>();

  auto taskList = MakeTaskList(*state);
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
      });
      state->selectedTask = static_cast<int>(state->tasks.size()) - 1;
    }
    state->draftTask.clear();
    state->showInput = false;
  };

  auto cancelInput = [state] {
    state->draftTask.clear();
    state->showInput = false;
  };

  auto renderer = Renderer(eventTarget, [=] {
    Element mainWindow = vbox({
        taskList->Render() | flex,
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
    if (event == Event::Return) {
      ToggleSelectedTaskCompletion(*state);
      return true;
    }
    if (event == Event::Character('p')) {
      ToggleSelectedTaskProgress(*state);
      return true;
    }
    if (event == Event::Character('d')) {
      ToggleSelectedTaskDeprecated(*state);
      return true;
    }
    if (event == Event::Character('q')) {
      quit();
      return true;
    }

    return false;
  });
}
