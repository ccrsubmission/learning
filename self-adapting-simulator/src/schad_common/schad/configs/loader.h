#ifndef LOADER_H
#define LOADER_H

#include <nlohmann/json.hpp>
#include <boost/filesystem.hpp>
#include <schad/common.h>

namespace schad {

using json = nlohmann::json;
namespace filesystem = boost::filesystem;
using path = filesystem::path;

class Loader {
    explicit Loader(path working_dir) 
        : working_dir_{working_dir} {
    }

public:
    template<class Func>
    auto load(Func f, json const& cfg) const {
        auto invoke = [f] (json const& cfg, Loader const& loader) {
            if constexpr (std::is_invocable_v<Func, json const&, Loader const&>) {
                return f(cfg, loader);
            } else {
                return f(cfg);
            }
        };
        if (cfg.is_string()) {
            auto file = path(cfg.get<string>());
            if (!file.is_absolute()) {
                file = working_dir_ / file;
            }
            return invoke(read_json(file), Loader{file.parent_path()});
        }
        return invoke(cfg, *this);
    }

    template<class Func>
    static auto load_from_dir(Func f, json const& cfg, 
            path working_dir = filesystem::current_path()) {
        return Loader{working_dir}.load(f, cfg);
    }

private:
    static auto read_json(path file) {
        if (!exists(file)) {
            throw std::invalid_argument("JSON file does not exist: " + file.string()); 
        }
        schad::json config{};
        std::ifstream is(file.native());
        is >> config;
        return config;
    }

private:
    path working_dir_;
};

}

#endif // LOADER_H
