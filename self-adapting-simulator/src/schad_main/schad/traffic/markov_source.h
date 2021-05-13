#ifndef MARKOV_SOURCE_H
#define MARKOV_SOURCE_H

#include <schad/traffic/source.h>
#include <schad/traffic/markov_source_parameters.h>

namespace schad {

auto create_markov_source(MarkovSourceParameters params) -> unique_ptr<SourceFactory>;  

}

#endif
