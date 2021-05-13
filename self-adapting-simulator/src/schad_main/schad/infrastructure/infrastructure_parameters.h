#ifndef INFRASTRUCTURE_PARAMETERS_H
#define INFRASTRUCTURE_PARAMETERS_H

#include <schad/common.h>

namespace schad {

struct InfrastructureParameters {
public:
    InfrastructureParameters() 
        : batch_size_{}, buffer_size_{10}, num_simulated_{0} {
    }

    auto& set_buffer_size(uint64_t buffer_size) {
        buffer_size_ = buffer_size;
        return *this;
    }

    auto buffer_size() const {
        return buffer_size_;
    }

    auto& set_batch_size(size_t batch_size) {
        batch_size_ = batch_size;
        return *this;
    }

    auto batch_size() const {
        return batch_size_;
    }

    auto& set_num_simulated(size_t num_simulated) {
        num_simulated_ = num_simulated;
        return *this;
    }

    auto num_simulated() const {
        return num_simulated_;
    }

private:
    size_t batch_size_;
    size_t buffer_size_;
    size_t num_simulated_;
};

}

#endif // INFRASTRUCTURE_PARAMETERS_H
