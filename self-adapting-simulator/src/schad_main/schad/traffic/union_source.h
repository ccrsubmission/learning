#ifndef UNION_SOURCE_H
#define UNION_SOURCE_H

#include <schad/traffic/source.h>

namespace schad {

auto create_union_source(vector<unique_ptr<SourceFactory>> srcs) -> unique_ptr<SourceFactory>;

}

#endif // UNION_SOURCE_H
