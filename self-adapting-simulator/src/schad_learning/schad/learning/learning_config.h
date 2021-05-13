#ifndef LEARNING_CONFIG_H
#define LEARNING_CONFIG_H

#include <schad/learning/learning_method.h>
#include <schad/learning/arm_identification_method.h>
#include <schad/configs/loader.h>

namespace schad {

struct unknown_learning_method_exception : std::invalid_argument {
    explicit unknown_learning_method_exception(string const& name)
        : invalid_argument("unknown schad_learning method: " + name) {
    }
};

struct unknown_arm_identification_method_exception : std::invalid_argument {
    explicit unknown_arm_identification_method_exception(string const& name)
        : invalid_argument("unknown arm identification method: " + name) {
    }
};

auto load_learning_method(json const& cfg, Loader const& loader) 
    -> shared_ptr<learning::LearningMethodFactory>;

auto load_arm_identification_method(json const& cfg, Loader const& loader) 
    -> shared_ptr<learning::ArmIdentificationMethodFactory>;

}

#endif // LEARNING_CONFIG_H
