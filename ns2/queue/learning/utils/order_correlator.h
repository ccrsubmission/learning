#ifndef NS_ORDER_CORRELATOR_H
#define NS_ORDER_CORRELATOR_H

#include <optional>
#include <vector>
#include <array>

class OrderCorrelator {
public:
    OrderCorrelator(size_t min_samples);

    void add_sample(double a, double b);

    auto get_result() const -> std::optional<double>;
private:
    static auto get_correlation(
            std::vector<double> const& xs,
            std::vector<double> const& ys) -> double;

private:
    size_t const min_samples_;

    std::array<std::vector<double>, 2> samples_;
};

#endif // NS_ORDER_CORRELATOR_H
