#include <iomanip>
#include <sstream>
#include <memory>
#include <chrono>

namespace tqgen {
namespace time {

auto parse(const std::string &str, const std::string &format) {
    std::istringstream iss{str};
    std::tm tm{};
    if (!(iss >> std::get_time(&tm, format.c_str())))
        throw std::invalid_argument("get_time");
    
    return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

} // namespace time
} // namespace tqgen
