#include <ftxui/component/app.hpp>

#include "TodoApp.h"

int main() {
  auto screen = ftxui::App::Fullscreen();
  screen.Loop(MakeTodoApp(screen.ExitLoopClosure()));
  return 0;
}
