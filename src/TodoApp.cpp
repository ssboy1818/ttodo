#include "TodoApp.h"

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppState.h"
#include "DeleteConfirmationWindow.h"
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

bool HasSelectedTask(const AppState &state) {
  return state.selectedTask >= 0 &&
         state.selectedTask < static_cast<int>(state.tasks.size());
}

int TaskStatusSortRank(TaskStatus status) {
  return static_cast<int>(status);
}

void RestoreSelectedTask(AppState &state, int selectedTaskId) {
  for (size_t index = 0; index < state.tasks.size(); ++index) {
    if (state.tasks[index].id == selectedTaskId) {
      state.selectedTask = static_cast<int>(index);
      return;
    }
  }

  state.selectedTask =
      std::clamp(state.selectedTask, 0,
                 std::max(0, static_cast<int>(state.tasks.size()) - 1));
}

void SortTasksByStatus(AppState &state) {
  const int selectedTaskId =
      HasSelectedTask(state)
          ? state.tasks[static_cast<size_t>(state.selectedTask)].id
          : 0;

  std::ranges::stable_sort(state.tasks, [](const Task &left,
                                           const Task &right) {
    return TaskStatusSortRank(left.status) < TaskStatusSortRank(right.status);
  });

  RestoreSelectedTask(state, selectedTaskId);
}

void ClampSelectedTask(AppState &state) {
  state.selectedTask =
      std::clamp(state.selectedTask, 0,
                 std::max(0, static_cast<int>(state.tasks.size()) - 1));
}

void ChangeTaskStatus(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

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

  if (state.showGroupedTasks) {
    SortTasksByStatus(state);
  }
}

void DeleteSelectedTask(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  state.tasks.erase(state.tasks.begin() + state.selectedTask);
  ClampSelectedTask(state);
}

void ToggleTaskDeprecated(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  Task &task = state.tasks[static_cast<size_t>(state.selectedTask)];

  task.status = TaskStatus::Deprecated;
  task.statusChangedAt = std::chrono::system_clock::now();

  if (state.showGroupedTasks) {
    SortTasksByStatus(state);
  }
}

} // namespace

ftxui::Component MakeTodoApp(ftxui::Closure quit) {
  using namespace ftxui;

  auto state = std::make_shared<AppState>();

  auto taskList = MakeTaskList(*state);
  auto taskDetails = MakeTaskDetails(*state);
  auto inputWindow = MakeTaskInputWindow(*state);
  auto deleteConfirmationWindow = MakeDeleteConfirmationWindow(*state);
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
          .id = state->nextTaskId++,
          .status = TaskStatus::Created,
          .createdAt = std::chrono::system_clock::now(),
      });
      state->selectedTask = static_cast<int>(state->tasks.size()) - 1;
      if (state->showGroupedTasks) {
        SortTasksByStatus(*state);
      }
    }
    state->draftTask.clear();
    state->showInput = false;
  };

  auto cancelInput = [state] {
    state->draftTask.clear();
    state->showInput = false;
  };

  auto cancelDelete = [state] { state->showDeleteConfirmation = false; };

  auto confirmDelete = [state] {
    DeleteSelectedTask(*state);
    state->showDeleteConfirmation = false;
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

    if (!state->showInput && !state->showDeleteConfirmation) {
      return mainWindow;
    }

    Element modal = state->showInput ? inputWindow->Render()
                                     : deleteConfirmationWindow->Render();

    return dbox({
        mainWindow | dim,
        modal | clear_under | center | vcenter,
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

    if (state->showDeleteConfirmation) {
      if (event == Event::Escape) {
        cancelDelete();
        return true;
      }
      if (event == Event::Return) {
        confirmDelete();
        return true;
      }
      return true;
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
      if (state->showGroupedTasks) {
        SortTasksByStatus(*state);
      }
      return true;
    }
    if (event == Event::Character(' ')) {
      ChangeTaskStatus(*state);
      return true;
    }
    if (event == Event::Character('d')) {
      ToggleTaskDeprecated(*state);
      return true;
    }
    if (event == Event::CtrlD) {
      if (HasSelectedTask(*state)) {
        state->showDeleteConfirmation = true;
      }
      return true;
    }
    if (event == Event::Character('q')) {
      quit();
      return true;
    }

    return false;
  });
}
