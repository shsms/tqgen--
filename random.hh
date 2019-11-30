#include <array>
#include <cstdint>

namespace tqgen {
class rand {
    constexpr static std::uint64_t lce_a = 48271, lce_c = 0,
                                   lce_m = (long(1) << 31) - 1;

    constexpr static auto time_from_string(const char *str, int offset) {
        return static_cast<std::uint64_t>(str[offset] - '0') * 10 +
               static_cast<std::uint64_t>(str[offset + 1] - '0');
    }

    std::uint64_t seed;

  public:
    constexpr rand(const std::uint64_t &seed) : seed(seed) {}

    constexpr static auto get_seed() {
        auto t = __TIME__;
        return time_from_string(t, 0) * 60 * 60 + time_from_string(t, 3) * 60 +
               time_from_string(t, 6);
    }

    constexpr std::uint64_t uniform() {
        this->seed = ((lce_a * this->seed + lce_c) % lce_m);
        return this->seed;
    }

    constexpr double uniform_double() {
        auto dst = uniform();
        return static_cast<double>(dst) / lce_m;
    }

    template <typename T> constexpr T uniform(T min, T max) {
        return static_cast<T>(this->uniform_double() * (max - min) + min);
    }

    template <typename T, std::size_t sz>
    constexpr auto uniform_distribution(T min, T max) {
        std::array<T, sz> dst{};
        for (auto &el : dst)
            el = static_cast<T>(this->uniform_double() * (max - min) + min);

        return dst;
    }

    constexpr auto normal() {
        double val = 0;
        for (std::size_t ii = 0; ii < 12; ii++)
            val += this->uniform_double();
        return val - 6;
    }

    constexpr auto normal(double min, double max) {
        auto mean = (max - min) / 2.0 + min,
             sd = (max - min) / (2.0 * 3.0); // 3.0 -> max sd
        auto res = this->normal() * sd + mean;
	if (res < min) {
	    res = min;
	}
	if (res > max) {
	    res = max;
	}
	return res;
    }

    template <typename T, std::size_t sz, std::size_t irwin_numbers = 12>
    constexpr auto normal_distribution(double mean, double sigma) {
        std::array<T, sz> dst{};
        for (auto &el : dst) {
            double val = 0;
            for (std::size_t i = 0; i < irwin_numbers; ++i)
                val += this->uniform_double();

            // UPDATE: bug here. Should be std::sqrt((irwin_numbers / 12)),
            // but std::sqrt is not constexpr by default,
            // and I don't want to use any external libraries here by now
            el = val / (irwin_numbers / 12) - irwin_numbers / 2;
        }

        return dst;
    }
};
} // namespace tqgen
