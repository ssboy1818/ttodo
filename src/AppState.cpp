#include "AppState.h"

#include <algorithm>

bool HasSelectedTask(const AppState &state) {
  return state.selectedTask >= 0 &&
         state.selectedTask < static_cast<int>(state.tasks.size());
}

void ClampSelectedTask(AppState &state) {
  state.selectedTask =
      std::clamp(state.selectedTask, 0,
                 std::max(0, static_cast<int>(state.tasks.size()) - 1));
}

void ClampSelectedTag(AppState &state) {
  state.selectedTag =
      std::clamp(state.selectedTag, 0,
                 std::max(0, static_cast<int>(state.tags.size()) - 1));
}
