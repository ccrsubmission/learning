#ifndef POISSON_SOURCE_H
#define POISSON_SOURCE_H

#include <schad/traffic/source.h>
#include <schad/traffic/poisson_source_parameters.h>

namespace schad {

auto create_poisson_source(PoissonSourceParameters params) -> unique_ptr<SourceFactory>;  

}

#endif // POISSON_SOURCE_H
