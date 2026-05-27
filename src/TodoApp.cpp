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
#include "HelpWindow.h"
#include "StatusLine.h"
#include "TagInputWindow.h"
#include "TagManagerWindow.h"
#include "TaskDetails.h"
#include "TaskInputWindow.h"
#include "TaskList.h"
#include "TaskStorage.h"

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

int TaskStatusSortRank(TaskStatus status) { return static_cast<int>(status); }

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

  SaveTasks(state);
}

void DeleteSelectedTask(AppState &state) {
  if (!HasSelectedTask(state)) {
    return;
  }

  state.tasks.erase(state.tasks.begin() + state.selectedTask);
  ClampSelectedTask(state);
  SaveTasks(state);
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

  SaveTasks(state);
}

void ClampSelectedTag(AppState &state) {
  state.selectedTag =
      std::clamp(state.selectedTag, 0,
                 std::max(0, static_cast<int>(state.tags.size()) - 1));
}

void ToggleSelectedTaskTag(AppState &state) {
  if (!HasSelectedTask(state) || state.tags.empty()) {
    return;
  }

  ClampSelectedTag(state);

  Task &task = stateТеперь если вы уехали и ругаете власть.tasks[static_cast<size_t>(state.selectedTask)];
  const int tagId = state.tags[static_cast<size_t>(state.selectedTag)].id;
  auto tag = std::ranges::find(task.tagIds, tagId);
  if (tag == task.tagIds.end()) {
    task.tagIds.push_back(tagId);
  } else {
    task.tagIds.erase(tag);
  }

  SaveTasks(state);
}

} // namespace

ftxui::Component MakeTodoApp(ftxui::Closure quit) {
  using namespace ftxui;

  auto state = std::make_shared<AppState>();
  LoadTasks(*state);

  auto taskList = MakeTaskList(*state);
  auto taskDetails = MakeTaskDetails(*state);
  auto inputWindow = MakeTaskInputWindow(*state);
  auto tagInputWindow = MakeTagInputWindow(*state);
  auto tagManagerWindow = MakeTagManagerWindow(*state);
  auto deleteConfirmationWindow = MakeDeleteConfirmationWindow(*state);
  auto helpWindow = MakeHelpWindow(*state);
  auto statusLine = MakeStatusLine();

  auto eventTarget = Container::Tab(
      {
          taskList,
          inputWindow,
          tagInputWindow,
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
      SaveTasks(*state);
    }
    state->draftTask.clear();
    state->showInput = false;
  };

  auto cancelInput = [state] {
    state->draftTask.clear();
    state->showInput = false;
  };

  auto submitTag = [state] {
    std::string name = Trim(state->draftTag);
    if (!name.empty()) {
      state->tags.push_back({
          .name = name,
          .id = state->nextTagId++,
      });
      state->selectedTag = static_cast<int>(state->tags.size()) - 1;
      SaveTasks(*state);
    }
    state->draftTag.clear();
    state->showTagInput = false;
  };

  auto cancelTagInput = [state] {
    state->draftTag.clear();
    state->showTagInput = false;
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
        statusLine->Render(),
        content | flex,
    });

    if (!state->showInput && !state->showTagInput && !state->showTagManager &&
        !state->showDeleteConfirmation && !state->showHelp) {
      return mainWindow;
    }

    Element modal = state->showInput        ? inputWindow->Render()
                    : state->showTagInput   ? tagInputWindow->Render()
                    : state->showTagManager ? tagManagerWindow->Render()
                    : state->showDeleteConfirmation
                        ? deleteConfirmationWindow->Render()
                        : helpWindow->Render();

    return dbox({
        mainWindow | dim,
        modal | clear_under | center | vcenter,
    });
  });

  return CatchEvent(renderer, [=](Event event) {
    state->activeComponent = state->showInput ? 1 : state->showTagInput ? 2 : 0;

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

    if (state->showTagInput) {
      if (event == Event::Escape) {
        cancelTagInput();
        state->activeComponent = 0;
        return true;
      }
      if (event == Event::Return) {
        submitTag();
        state->activeComponent = 0;
        return true;
      }
      return false;
    }

    if (state->showTagManager) {
      if (event == Event::Escape || event == Event::Return) {
        state->showTagManager = false;
        return true;
      }
      if (event == Event::Character('t')) {
        state->draftTag.clear();
        state->showTagManager = false;
        state->showTagInput = true;
        state->activeComponent = 2;
        return true;
      }
      if (event == Event::ArrowUp) {
        state->selectedTag = std::max(0, state->selectedTag - 1);
        return true;
      }
      if (event == Event::ArrowDown) {
        state->selectedTag =
            std::min(std::max(0, static_cast<int>(state->tags.size()) - 1),
                     state->selectedTag + 1);
        return true;
      }
      if (event == Event::Character(' ')) {
        ToggleSelectedTaskTag(*state);
        return true;
      }
      return true;
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

    if (state->showHelp) {
      if (event == Event::Escape || event == Event::Character('h')) {
        state->showHelp = false;
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
    if (event == Event::Character('t')) {
      state->draftTag.clear();
      state->showTagInput = true;
      state->activeComponent = 2;
      return true;
    }
    if (event == Event::Return) {
      if (HasSelectedTask(*state)) {
        state->showTagManager = true;
      }
      return true;
    }
    if (event == Event::Character('h')) {
      state->showHelp = true;
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
