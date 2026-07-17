#pragma once

#include <numeric>
#include <ranges>

#include <opencv2/core/types.hpp>

namespace utils {

template <class R>
requires std::ranges::input_range<R>
auto centroid(const R& rng) {
    using P = std::remove_cvref_t<std::ranges::range_reference_t<R>>;
    using T = typename P::value_type;

    const T sum_x = std::accumulate(
        std::begin(rng), std::end(rng), T{},
        [](T acc, const auto& p) { return acc + static_cast<T>(p.x); });
    const T sum_y = std::accumulate(
        std::begin(rng), std::end(rng), T{},
        [](T acc, const auto& p) { return acc + static_cast<T>(p.y); });

    const auto n = std::ranges::size(rng);
    if (n == 0) {
        return cv::Point_<T>(T{}, T{});
    }

    return cv::Point_<T>(sum_x / static_cast<T>(n), sum_y / static_cast<T>(n));
}

}  // namespace utils
