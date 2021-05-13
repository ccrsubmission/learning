#ifndef LEARNING_METHOD_FACTORY_HELPER_H
#define LEARNING_METHOD_FACTORY_HELPER_H

#include <schad/learning/learning_method.h>

namespace schad::learning {

template<class Instantiator>
class LearningMethodFactoryHelper : public LearningMethodFactory {
public:
    LearningMethodFactoryHelper(Instantiator instantiator, json const& j) 
        : instantiator_{std::move(instantiator)}, json_(j) {
    }

    auto instantiate(shared_ptr<rng_t> rng, size_t num_arms) const 
        -> unique_ptr<LearningMethod> override {
        return instantiator_(std::move(rng), num_arms);
    }

    void to_json(json& j) const override {
        j = json_;
    }
    
private:
    Instantiator const instantiator_;
    json const json_;
};

template<class Instantiator>
auto create_learning_factory(Instantiator instantiator, json const& j) 
    -> unique_ptr<LearningMethodFactory> {
    return make_unique<LearningMethodFactoryHelper<Instantiator>>(std::move(instantiator), j);
}

}


#endif // LEARNING_METHOD_FACTORY_HELPER_H
