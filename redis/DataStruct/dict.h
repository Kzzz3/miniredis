#pragma once
#include <cstdint>
#include <unordered_map>

#include "sds.h"
#include "../DataType/redisobj.h"

using Dict = std::unordered_map<Sds*, RedisObj*>;