#ifndef STATISTICS_H
#define STATISTICS_H

#include <schad/common.h>

namespace schad {

struct Statistics {
    Statistics(vector<vector<double>> rewards, 
               vector<vector<double>> totals, 
               vector<vector<vector<size_t>>> arms)
        : rewards{std::move(rewards)}, totals{std::move(totals)}, 
          arms{std::move(arms)} {
    }

    vector<vector<double>> const rewards;
    vector<vector<double>> const totals;
    vector<vector<vector<size_t>>> const arms;
};

inline void to_json(json& j, Statistics const& stats) {
    j = {{"rewards", stats.rewards},
         {"arms", stats.arms}, 
         {"totals", stats.totals}}; 
}

}

#endif // STATISTICS_H
