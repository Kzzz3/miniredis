export module dict;

import std;

union dictVal
{
    void* val;
    double dnum;
    std::int64_t i64num;
    std::uint64_t u64num;
};

export using dict = std::unordered_map<void*, dictVal>;