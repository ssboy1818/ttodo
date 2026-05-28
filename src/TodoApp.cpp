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
  return state.activeComponent != ActiveComponent::TaskList;
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

    if (!HasOpenModal(*state)) {
      return mainWindow;
    }

    Element modal =
        state->activeComponent == ActiveComponent::TaskInput
            ? inputWindow->Render()
        : state->activeComponent == ActiveComponent::TagInput
            ? tagInputWindow->Render()
        : state->activeComponent == ActiveComponent::TagManager
            ? tagManagerWindow->Render()
        : state->activeComponent == ActiveComponent::DeleteConfirmation
            ? deleteConfirmationWindow->Render()
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
      state->activeComponent = ActiveComponent::TaskInput;
      return true;
    }
    if (event == Event::Character('t')) {
      state->draftTag.clear();
      state->activeComponent = ActiveComponent::TagInput;
      return true;
    }
    if (event == Event::Return) {
      if (HasSelectedTask(*state)) {
        RefreshTagLabels(*state);
        state->activeComponent = ActiveComponent::TagManager;
      }
      return true;
    }
    if (event == Event::Character('h')) {
      state->activeComponent = ActiveComponent::Help;
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
