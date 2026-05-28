#include "TodoApp.h"

#include <memory>

#include <ftxui/component/component.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/dom/elements.hpp>

#include "AppActions.h"
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

bool HasOpenModal(const AppState &state) {
  return state.showInput || state.showTagInput || state.showTagManager ||
         state.showDeleteConfirmation || state.showHelp;
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
          tagManagerWindow,
          deleteConfirmationWindow,
          helpWindow,
      },
      &state->activeComponent);

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

    Element modal = state->showDeleteConfirmation
                        ? deleteConfirmationWindow->Render()
                    : state->showInput      ? inputWindow->Render()
                    : state->showTagInput   ? tagInputWindow->Render()
                    : state->showTagManager ? tagManagerWindow->Render()
                                            : helpWindow->Render();

    return dbox({
        mainWindow | dim,
        modal | clear_under | center | vcenter,
    });
  });

  return renderer | CatchEvent([=](Event event) {
    if (HasOpenModal(*state)) {
      return false;
    }

    if (event == Event::Character('n')) {
      state->draftTask.clear();
      state->showInput = true;
      state->activeComponent = ActiveComponent::TaskInput;
      return true;
    }
    if (event == Event::Character('t')) {
      state->draftTag.clear();
      state->showTagInput = true;
      state->activeComponent = ActiveComponent::TagInput;
      return true;
    }
    if (event == Event::Return) {
      if (HasSelectedTask(*state)) {
        state->showTagManager = true;
        state->activeComponent = ActiveComponent::TagManager;
      }
      return true;
    }
    if (event == Event::Character('h')) {
      state->showHelp = true;
      state->activeComponent = ActiveComponent::Help;
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
        state->deleteTarget = DeleteTarget::Task;
        state->deleteReturnComponent = ActiveComponent::TaskList;
        state->showDeleteConfirmation = true;
        state->activeComponent = ActiveComponent::DeleteConfirmation;
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
