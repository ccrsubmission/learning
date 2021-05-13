#ifndef MARKOV_SOURCE_PARAMETERS_H
#define MARKOV_SOURCE_PARAMETERS_H

#include <map>

namespace schad {

struct MarkovState {
    MarkovState(std::string name, std::unique_ptr<SourceFactory> src, bool is_start)
        : name_{name}, src_{std::move(src)}, is_start_{is_start}, tr_{} {
    }

    MarkovState(MarkovState&&) = default;
    MarkovState& operator=(MarkovState&&) = default;

    auto& name() const {
        return name_;
    }

    auto& src() const {
        return src_;
    }

    auto is_start() const {
        return is_start_;
    }

    void add_transition(std::string const& to, double prob) {
        tr_[to] = prob;
    }

    auto& tr() const {
        return tr_;
    }
    
private:
    std::string name_;
    std::unique_ptr<SourceFactory> src_;
    bool is_start_;
    std::map<std::string, double> tr_;
};

struct MarkovSourceParameters {
    MarkovSourceParameters() 
        : states_{} {
    }

    void add_state(MarkovState state) {
        states_.emplace_back(std::move(state));
    }

    auto& states() const {
        return states_;
    }

private:
    vector<MarkovState> states_;
};

inline void to_json(json& j, MarkovState const& st) {
    j = { 
        {"is_start", st.is_start()},
        {"tr", st.tr()},
        {"src", *st.src()}
    };
}

inline void to_json(json& j, MarkovSourceParameters const& params) {
    j = {};
    for (auto const& st : params.states()) {
        j[st.name()] = st;
    }
}

}

#endif // MARKOV_SOURCE_PARAMETERS_H
