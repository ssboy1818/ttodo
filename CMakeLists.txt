cmake_minimum_required(VERSION 4.3)
project(ttodo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)
FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG 98c650d2ba6c22cb00bf3a3b3a7d1acd3d92ca1e
)
FetchContent_MakeAvailable(ftxui)

add_executable(ttodo
        src/Main.cpp
        src/AppActions.cpp
        src/AppState.cpp
        src/Task.cpp
        src/TodoApp.cpp
        src/TaskList.cpp
        src/TaskDetails.cpp
        src/TaskInputWindow.cpp
        src/TagInputWindow.cpp
        src/TagManagerWindow.cpp
        src/DeleteConfirmationWindow.cpp
        src/HelpWindow.cpp
        src/StatusLine.cpp
        src/TaskStorage.cpp
        src/MenuStyles.cpp
)

target_include_directories(ttodo PRIVATE include)

target_link_libraries(ttodo PRIVATE
        ftxui::screen
        ftxui::dom
        ftxui::component
)
