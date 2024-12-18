#pragma once
#include <fstream>
#include <unordered_map>
#include <functional>

#include <ylt/struct_pack.hpp>

#include "sds.h"

using std::function;
using std::ifstream;
using std::ofstream;
using std::unordered_map;

template <typename T> class HashTable : public unordered_map<Sds*, T>
{
public:
    template <typename ValSeriFunc>
    static void serialize_to(ofstream& ofs, HashTable* table, ValSeriFunc val_seri_func)
    {
        int size = table->size();
        struct_pack::serialize_to(ofs, size);
        for (auto& [key, value] : *table)
        {
            Sds::serialize_to(ofs, key);
            val_seri_func(ofs, value);
        }
    }

    template <typename ValDeseriFunc>
    static HashTable* deserialize_from(ifstream& ifs, ValDeseriFunc val_deseri_func)
    {
        auto expect_size = struct_pack::deserialize<int>(ifs);
        if (!expect_size.has_value())
            throw std::runtime_error("deserialize failed");
        int size = expect_size.value();

        HashTable<T>* table = Allocator::create<HashTable<T>>();
        for (int i = 0; i < size; i++)
        {
            Sds* key = Sds::deserialize_from(ifs);
            T value = val_deseri_func(ifs);
            table->insert({key, value});
        }
        return table;
    }
};