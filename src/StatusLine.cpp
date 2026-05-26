#include "StatusLine.h"

#include <chrono>
#include <ctime>
#include <iomanip>
#include <locale>
#include <sstream>
#include <string>

#include <ftxui/component/component.hpp>
#include <ftxui/dom/elements.hpp>

namespace {

std::string CurrentLocalizedDateTime() {
  const auto now = std::chrono::system_clock::now();
  const std::time_t rawTime = std::chrono::system_clock::to_time_t(now);
  std::tm localTime{};
  localtime_r(&rawTime, &localTime);

  std::ostringstream output;
  try {
    output.imbue(std::locale(""));
  } catch (const std::runtime_error &) {
  }

  output << std::put_time(&localTime, "%d %B %a %H:%M");
  return output.str();
}

} // namespace

ftxui::Component MakeStatusLine() {
  using namespace ftxui;

  return Renderer([] {
    return hbox({
               text("    "),
               text("ttodo") | bold,
               separatorEmpty(),
               filler(),
               text(CurrentLocalizedDateTime()),
               text("    "),
           }) |
           inverted;
  });
}
