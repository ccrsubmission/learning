#include "order_correlator.h"

#include <algorithm>
#include <numeric>
#include <map>
#include <cmath>

OrderCorrelator::OrderCorrelator(size_t min_samples)
    : min_samples_{min_samples}
    , samples_{}
{}

void OrderCorrelator::add_sample(double a, double b) {
    samples_[0].push_back(round(a * 100));
    samples_[1].push_back(round(b * 100));
}

auto OrderCorrelator::get_result() const -> std::optional<double> {
    if (samples_[0].size() < min_samples_) {
        return std::nullopt;
    } else {
        return get_correlation(samples_[0], samples_[1]);
    }
}


auto OrderCorrelator::get_correlation(
        std::vector<double> const& xs,
        std::vector<double> const& ys
        ) -> double 
{
    auto num_concordant = 0;
    auto num_discordant = 0;
    std::map<double, size_t> x_groups;
    std::map<double, size_t> y_groups;
    for (auto i = 0; i < int(xs.size()); i++) {
        auto const xi = xs[i];
        x_groups[xi]++;
        auto const yi = ys[i];
        y_groups[yi]++;
        for (auto j = i + 1; j < int(ys.size()); j++) {
            auto const xj = xs[j];
            auto const yj = ys[j];

            if ((xi < xj && yi < yj) || (xi > xj && yi > yj)) {
                num_concordant++;
            }
            if ((xi > xj && yi < yj) || (xi < xj && yi > yj)) {
                num_discordant++;
            }
        }
    }
    int const cnt_x = xs.size() * (xs.size() - 1) / 2;
    int const cnt_y = ys.size() * (ys.size() - 1) / 2;

    auto ties_x = 0;
    for (auto [x, cnt] : x_groups) {
        ties_x += cnt * (cnt - 1) / 2;
    }
    auto ties_y = 0;
    for (auto [y, cnt] : y_groups) {
        ties_y += cnt * (cnt - 1) / 2;
    }

    if (cnt_y == ties_y) {
        return ties_x / (double) cnt_x;
    }
    
    if (cnt_x == ties_x) {
        return ties_y / (double) cnt_y;
    }

    auto result = (num_concordant - num_discordant) 
        / (double) sqrt((cnt_x - ties_x) * (cnt_y - ties_y));
    if (result != result) {
        throw std::logic_error("NaN correlation is bad");
    }
    return result;
}
