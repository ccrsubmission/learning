#ifndef NULL_SOURCE_H
#define NULL_SOURCE_H

#include <schad/traffic/source.h>

namespace schad {

auto create_null_source() -> unique_ptr<SourceFactory>;  
}

#endif // NULL_SOURCE
