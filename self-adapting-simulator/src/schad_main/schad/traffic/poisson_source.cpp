#include "poisson_source.h"

#include <iostream>

#include <random>
#include <exception>
#include <schad/packet/packet_builder.h>

namespace {
using namespace schad;

class PoissonSource : public Source {
public:
    PoissonSource(PoissonSourceParameters params, shared_ptr<rng_t> rng)
        : work_dis_{params.min_work(), params.max_work()},
          value_dis_{params.min_value(), params.max_value()},
          count_dis_{params.rate()}, slack_dis_{},
          rng_{std::move(rng)} {
        if (params.min_slack().has_value()) {
            slack_dis_ = std::uniform_real_distribution<double>(
                    *params.min_slack(), *params.max_slack()
            );
        }
    }

    auto next(uint32_t time) -> vector<unique_ptr<Packet>> override {
        vector<unique_ptr<Packet>> result{};
        std::generate_n(back_inserter(result), count_dis_(*rng_), [this,time] {
            auto const work = work_dis_(*rng_);
            auto builder = PacketBuilder::create()
                .set_initial_processing(work)
                .set_value(value_dis_(*rng_))
                .set_time_of_arrival(time);
            if (slack_dis_.has_value()) { 
                builder.set_slack(round(work * (*slack_dis_)(*rng_)));
            }
            return builder.build();
        });
        return result;
    }
    
private:
    std::uniform_int_distribution<uint32_t> work_dis_;
    std::uniform_real_distribution<double> value_dis_;
    std::poisson_distribution<size_t> count_dis_;
    optional<std::uniform_real_distribution<double>> slack_dis_;
    shared_ptr<rng_t> rng_;
};

class PoissonSourceFactory : public SourceFactory {
public:
    PoissonSourceFactory(PoissonSourceParameters params)
        : params_{params} {
    }

    auto instantiate(shared_ptr<rng_t> rng) const -> unique_ptr<Source> override {
        return make_unique<PoissonSource>(params_, std::move(rng));
    }

    void to_json(json& j) const override {
        j = {{"type", "poisson"}, {"parameters", params_}};
    }
    
private:
    PoissonSourceParameters const params_;
};

} // namespace

auto schad::create_poisson_source(PoissonSourceParameters params) -> unique_ptr<SourceFactory> {
    if (params.min_slack().has_value() != params.max_slack().has_value()) {
        throw std::invalid_argument("Slack's min and max should be simultaneously defined!");
    }
    return make_unique<PoissonSourceFactory>(params);
}
