#pragma once

#include <string>

#include "AppState.h"

void RefreshTaskLabels(AppState &state);
void RefreshTagLabels(AppState &state);
void AddTask(AppState &state, std::string title);
void ChangeTaskStatus(AppState &state);
void DeleteSelectedTask(AppState &state);
void ToggleTaskDeprecated(AppState &state);

void AddTag(AppState &state, std::string name);
void DeleteSelectedTag(AppState &state);
void ToggleSelectedTaskTag(AppState &state);
