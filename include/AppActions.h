#pragma once

#include <string>

#include "AppState.h"

void AddTask(AppState &state, std::string title);
void ChangeTaskStatus(AppState &state);
void DeleteSelectedTask(AppState &state);
void ToggleTaskDeprecated(AppState &state);
void SortTasksByStatus(AppState &state);

void AddTag(AppState &state, std::string name);
void MoveSelectedTag(AppState &state, int delta);
void DeleteSelectedTag(AppState &state);
void ToggleSelectedTaskTag(AppState &state);
