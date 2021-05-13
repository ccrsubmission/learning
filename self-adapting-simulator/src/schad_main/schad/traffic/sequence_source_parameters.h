#ifndef SEQUENCE_SOURCE_PARAMETERS_H
#define SEQUENCE_SOURCE_PARAMETERS_H

#include <schad/common.h>
#include <exception>

namespace schad {

struct SequenceSourceParameters {
    SequenceSourceParameters() 
        : num_steps_{}, sources_{} {
    }

    void add_source(size_t num_steps, std::unique_ptr<SourceFactory> src) {
        if (num_steps <= 0) {
            throw std::invalid_argument("Number of time steps should be positive!");
        }
        num_steps_.emplace_back(num_steps);
        sources_.emplace_back(std::move(src));
    }

    auto& sources() const {
        return sources_;
    }

    auto& num_steps() const {
        return num_steps_;
    }

private:
    std::vector<size_t> num_steps_;
    std::vector<unique_ptr<SourceFactory>> sources_;
};

}

#endif // ON_OFF_SOURCE_PARAMETERS_H
