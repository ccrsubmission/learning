#ifndef ARM_IDENTIFICATION_METHOD_H
#define ARM_IDENTIFICATION_METHOD_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

struct ArmIdentificationMethod : public LearningMethod {
    virtual auto get_best() const -> optional<size_t> = 0;
};

struct ArmIdentificationMethodFactory {
    virtual auto instantiate(shared_ptr<rng_t> rng,
                             size_t       num_arms, 
                             size_t     time_limit) const
        -> unique_ptr<ArmIdentificationMethod> = 0;


    virtual void to_json(json& j) const = 0;

    ArmIdentificationMethodFactory() = default;
    ArmIdentificationMethodFactory(ArmIdentificationMethodFactory const&) 
        = delete;
    ArmIdentificationMethodFactory& operator=(
            ArmIdentificationMethodFactory const&
        ) = delete;
};

inline void to_json(json& j, ArmIdentificationMethodFactory const& learning) {
    learning.to_json(j);
}

}

#endif // ARM_IDENTIFICATION_METHOD_H
