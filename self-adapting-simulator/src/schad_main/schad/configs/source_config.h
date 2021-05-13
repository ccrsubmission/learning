#ifndef SOURCE_CONFIG_H
#define SOURCE_CONFIG_H

#include <schad/traffic/source.h>
#include <schad/configs/loader.h>

namespace schad {

struct unknown_source_exception : std::invalid_argument {
    unknown_source_exception(string const& source_name)
        : invalid_argument("unknown source: " + source_name) {
    }
};

auto load_source(json const& cfg, Loader const& loader) 
    -> unique_ptr<SourceFactory>; 

}

#endif // SOURCE_CONFIG_H
