#pragma once
#include <cstdint>
#include <unordered_map>

#include "sds.h"

union DictVal
{
    void* val;
    double dnum;
    std::int64_t i64num;
    uint64_t u64num;
};

using Dict = std::unordered_map<Sds*, DictVal>;