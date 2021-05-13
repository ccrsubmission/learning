#ifndef SEQUENCE_SOURCE_H
#define SEQUENCE_SOURCE_H

#include <schad/traffic/source.h>
#include <schad/traffic/sequence_source_parameters.h>

namespace schad {

auto create_sequence_source(SequenceSourceParameters params) -> unique_ptr<SourceFactory>;  

}

#endif
