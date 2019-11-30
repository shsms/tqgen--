#include <chrono>
#include <fmt/chrono.h>
#include <fmt/format.h>
#include <iomanip>
#include <memory>
#include <sstream>

namespace tqgen {
namespace time {

inline auto parse(const std::string &str, const std::string &format) {
    std::istringstream iss{str};
    std::tm tm{};
    if (!(iss >> std::get_time(&tm, format.c_str())))
        throw std::invalid_argument("get_time");
    return std::chrono::system_clock::from_time_t(std::mktime(&tm) - timezone);
}

inline auto to_date(const std::chrono::system_clock::time_point &tp) {
    auto timet = std::chrono::system_clock::to_time_t(tp);
    return fmt::format("{:%Y%m%d}", *std::localtime(&timet));
}

inline auto to_timems(const std::chrono::system_clock::time_point &tp) {
    auto msec = std::chrono::time_point_cast<std::chrono::milliseconds>(tp)
                    .time_since_epoch()
                    .count();
    auto sec = (msec / 1000) % 60;
    auto min = (msec / 1000 / 60) % 60;
    auto hour = (msec / 1000 / 60 / 60) % 24;
    return fmt::format("{:0>2}{:0>2}{:0>2}.{:0>3}", hour, min, sec,
                       msec % 1000);
}

} // namespace time
} // namespace tqgen
