#pragma once
#include <unordered_map>

#include "sds.h"
#include "../DataType/redisobj.h"

template <typename T>
using HashTable = std::unordered_map<Sds*, T>;